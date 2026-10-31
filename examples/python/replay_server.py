"""SIMULATED local HTTP device for desktop failure testing, never real telemetry."""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import time


def make_handler(fail_every=0, delay=0.0):
    class Handler(BaseHTTPRequestHandler):
        count = 0

        def do_GET(self):
            type(self).count += 1
            time.sleep(delay)
            failed = bool(fail_every and type(self).count % fail_every == 0)
            payload = {"simulated": True, "seq": type(self).count, "valid": not failed,
                       "temperature_c": None if failed else 23.5, "humidity_pct": None if failed else 48.0}
            self.send_response(503 if failed else 200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            try:
                self.wfile.write(json.dumps(payload).encode())
            except (BrokenPipeError, ConnectionResetError):
                pass  # expected when a timeout experiment disconnects
    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8090)
    parser.add_argument("--fail-every", type=int, default=0)
    parser.add_argument("--delay", type=float, default=0)
    args = parser.parse_args()
    if args.fail_every < 0 or args.delay < 0:
        parser.error("delay/fail-every must be non-negative")
    server = ThreadingHTTPServer(("127.0.0.1", args.port), make_handler(args.fail_every, args.delay))
    print(f"SIMULATION http://127.0.0.1:{args.port}/api/status", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
