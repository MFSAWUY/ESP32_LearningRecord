import importlib.util
import math
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import unittest
from http.server import ThreadingHTTPServer
from urllib.error import HTTPError

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from math_lab import PID, average_current, generate, lowpass, rc_voltage
from network_tool import http_get
from protocol import parse_record
from replay_server import make_handler


class Mathematics(unittest.TestCase):
    def test_rc_initial_and_time_constant(self):
        self.assertEqual(rc_voltage(0, 10000, 1e-6), 0)
        self.assertAlmostEqual(rc_voltage(.01, 10000, 1e-6), 3.3 * (1-math.exp(-1)))

    def test_filter_constant_converges_without_overshoot(self):
        value = 0
        for _ in range(100):
            value = lowpass(value, 1, .01, .1)
            self.assertGreaterEqual(value, 0)
            self.assertLessEqual(value, 1)
        self.assertAlmostEqual(value, 1, places=4)

    def test_alias_indistinguishable_samples(self):
        rows, _ = generate("alias")
        self.assertLess(max(abs(r[1]-r[2]) for r in rows), 1e-12)

    def test_power_boundaries_and_invalid_period(self):
        self.assertEqual(average_current(100, 1, 0, 60), 1)
        self.assertEqual(average_current(100, 1, 60, 60), 100)
        with self.assertRaises(ValueError):
            average_current(100, 1, 61, 60)

    def test_pid_saturation_recovers(self):
        controller = PID()
        for _ in range(1000):
            self.assertEqual(controller.update(100, 0, .01), 1)
        self.assertEqual(controller.integral, 0)
        self.assertEqual(controller.update(0, 1, .01), 0)


class Protocol(unittest.TestCase):
    def test_missing_sensor_and_extensibility(self):
        record = parse_record('{"valid":false,"temperature_c":null,"new_field":42}')
        self.assertIsNone(record["temperature_c"])
        self.assertEqual(record["new_field"], 42)

    def test_reject_bad_records(self):
        for value in ["[]", "garbage", '{"temperature_c":NaN}', '{"seq":true}', '{"valid":1}', b"\xff", " " * 4097]:
            with self.subTest(value=str(value)[:30]), self.assertRaises(ValueError):
                parse_record(value)


class Networking(unittest.TestCase):
    def test_http_success_then_failure(self):
        server = ThreadingHTTPServer(("127.0.0.1", 0), make_handler(fail_every=2))
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        try:
            url = f"http://127.0.0.1:{server.server_port}/api/status"
            self.assertTrue(http_get(url)["simulated"])
            with self.assertRaises(HTTPError) as raised:
                http_get(url)
            self.assertEqual(raised.exception.code, 503)
        finally:
            server.shutdown()
            server.server_close()
            thread.join()


@unittest.skipUnless(importlib.util.find_spec("serial"), "pyserial not installed")
class SerialIntegration(unittest.TestCase):
    def test_loopback_csv(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "capture.csv"
            result = subprocess.run([sys.executable, str(Path(__file__).resolve().parents[1] / "serial_tool.py"),
                                     "--port", "loop://", "--duration", "0.2", "--output", str(output),
                                     "--send", '{"seq":7,"temperature_c":21.5,"valid":true}'],
                                    capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("21.5", output.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
