"""Loopback-only browser dashboard for the low_voltage_controller project."""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
import urllib.error
import urllib.request
from urllib.parse import urlsplit

HTML = b'''<!doctype html><meta charset="utf-8"><title>ESP32 local dashboard</title>
<style>body{font:18px system-ui;max-width:760px;margin:3rem auto;background:#f5f7fa;color:#152a3a}button{padding:12px;margin:8px}pre{white-space:pre-wrap;background:white;padding:20px}</style>
<h1>ESP32 local dashboard</h1><p>Local control; refresh every 2 seconds. A network failure does not confirm an actuator state.</p>
<button onclick="command(1)">LED ON</button><button onclick="command(0)">LED OFF</button>
<pre id="status">Connecting...</pre><script>
async function refresh(){try{let r=await fetch('/status');let t=await r.text();document.getElementById('status').textContent=t}catch(e){document.getElementById('status').textContent=String(e)}}
async function command(on){await fetch('/led?on='+on,{method:'POST',headers:{'X-Handbook':'1'}});refresh()}
refresh();setInterval(refresh,2000);</script>'''


def make_handler(device, token):
    class Handler(BaseHTTPRequestHandler):
        def reply(self, code, data, mime="application/json; charset=utf-8"):
            self.send_response(code)
            self.send_header("Content-Type", mime)
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.end_headers()
            self.wfile.write(data)

        def forward(self, path, method="GET"):
            request = urllib.request.Request(device + path, data=b"" if method == "POST" else None,
                                             headers={"Authorization": "Bearer " + token}, method=method)
            try:
                with urllib.request.urlopen(request, timeout=3) as response:
                    body = response.read(65537)
                    if len(body) > 65536:
                        raise ValueError("response too large")
                    self.reply(response.status, body)
            except (OSError, ValueError, urllib.error.URLError) as exc:
                self.reply(502, json.dumps({"error": str(exc)}).encode())

        def do_GET(self):
            if self.path == "/":
                self.reply(200, HTML, "text/html; charset=utf-8")
            elif self.path == "/status":
                self.forward("/api/status")
            else:
                self.reply(404, b'{"error":"not found"}')

        def do_POST(self):
            # A custom header without CORS permission prevents a foreign web
            # page from issuing a simple cross-origin form control request.
            expected_host = f"127.0.0.1:{self.server.server_port}"
            if self.headers.get("X-Handbook") != "1" or self.headers.get("Host") != expected_host:
                self.reply(403, b'{"error":"local browser request required"}')
                return
            if self.path not in ("/led?on=0", "/led?on=1"):
                self.reply(400, b'{"error":"on must be 0 or 1"}')
                return
            self.forward("/api" + self.path, "POST")
    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--device", required=True, help="http://ESP32-LAN-IP (fixed destination)")
    parser.add_argument("--port", type=int, default=8080)
    args = parser.parse_args()
    parsed = urlsplit(args.device)
    if parsed.scheme not in ("http", "https") or not parsed.hostname or parsed.path not in ("", "/") or parsed.query or parsed.fragment or parsed.username:
        parser.error("device must be an http(s) origin without path/query/user info")
    token = os.environ.get("ESP32_TOKEN", "")
    if not token:
        parser.error("set ESP32_TOKEN to match the firmware config first")
    server = ThreadingHTTPServer(("127.0.0.1", args.port), make_handler(args.device.rstrip("/"), token))
    print(f"Open http://127.0.0.1:{server.server_port} ; Ctrl+C to stop", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
