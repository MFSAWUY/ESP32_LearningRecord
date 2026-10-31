"""HTTP/MQTT client. Authentication secrets come from environment variables."""
import argparse
import json
import os
import ssl
import sys
import time
import urllib.error
import urllib.request


def http_get(url, token="", timeout=5):
    request = urllib.request.Request(url, headers={"Authorization": "Bearer " + token} if token else {})
    with urllib.request.urlopen(request, timeout=timeout, context=ssl.create_default_context()) as response:
        body = response.read(65537)
        if len(body) > 65536:
            raise ValueError("HTTP response too large")
        return json.loads(body.decode("utf-8"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="mode", required=True)
    http = sub.add_parser("http")
    http.add_argument("url")
    mqtt = sub.add_parser("mqtt")
    mqtt.add_argument("--host", required=True)
    mqtt.add_argument("--port", type=int, default=1883)
    mqtt.add_argument("--topic", default="handbook/+/telemetry")
    mqtt.add_argument("--duration", type=float, default=30)
    mqtt.add_argument("--publish", help="send this UTF-8 payload once to --topic")
    mqtt.add_argument("--tls", action="store_true")
    args = parser.parse_args()
    try:
        if args.mode == "http":
            print(json.dumps(http_get(args.url, os.environ.get("ESP32_TOKEN", "")), ensure_ascii=False, indent=2))
            return 0
        if args.duration <= 0:
            parser.error("duration must be positive")
        if args.publish is not None and any(char in args.topic for char in "+#"):
            parser.error("a publish topic cannot contain wildcards")
        import paho.mqtt.client as mqtt_client
        client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2)
        username = os.environ.get("MQTT_USERNAME")
        if username:
            client.username_pw_set(username, os.environ.get("MQTT_PASSWORD", ""))
        if args.tls:
            client.tls_set_context(ssl.create_default_context())
        client.reconnect_delay_set(min_delay=1, max_delay=30)
        state = {"connected": False, "published": False}

        def connected(client, userdata, flags, reason_code, properties):
            if reason_code.is_failure:
                print(f"Connection refused: {reason_code}", file=sys.stderr)
                return
            state["connected"] = True
            if args.publish is None:
                client.subscribe(args.topic, qos=1)
            elif not state["published"]:
                client.publish(args.topic, args.publish, qos=1, retain=False)
                state["published"] = True

        def message(client, userdata, msg):
            print(json.dumps({"topic": msg.topic, "payload": msg.payload.decode("utf-8", errors="replace"), "qos": msg.qos}, ensure_ascii=False), flush=True)

        client.on_connect = connected
        client.on_message = message
        client.connect_async(args.host, args.port, keepalive=30)
        client.loop_start()
        try:
            end = time.monotonic() + args.duration
            while time.monotonic() < end:
                time.sleep(.1)
        finally:
            client.disconnect()
            client.loop_stop()
        return 0 if state["connected"] else 1
    except KeyboardInterrupt:
        return 0
    except (OSError, ValueError, urllib.error.URLError) as exc:
        print(f"Network error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
