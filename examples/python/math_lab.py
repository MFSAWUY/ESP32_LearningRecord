"""Reproducible analytical exercises. All generated data are SIMULATED."""
import argparse
import csv
import math
from pathlib import Path


def rc_voltage(t: float, resistance: float, capacitance: float, supply: float = 3.3) -> float:
    if t < 0 or resistance <= 0 or capacitance <= 0:
        raise ValueError("t >= 0, R > 0 and C > 0 required")
    return supply * (-math.expm1(-t / (resistance * capacitance)))


def lowpass(previous: float, sample: float, dt: float, tau: float) -> float:
    if dt <= 0 or tau <= 0:
        raise ValueError("dt and tau must be positive")
    alpha = -math.expm1(-dt / tau)
    return previous + alpha * (sample - previous)


def average_current(active_ma: float, sleep_ma: float, active_s: float, period_s: float) -> float:
    """All currents MUST refer to the same supply voltage/measurement boundary."""
    if min(active_ma, sleep_ma, active_s) < 0 or period_s <= 0 or active_s > period_s:
        raise ValueError("invalid duty cycle/current")
    return (active_ma * active_s + sleep_ma * (period_s - active_s)) / period_s


class PID:
    """Discrete PID with derivative on measurement and conditional integration."""
    def __init__(self, kp=1.0, ki=0.4, kd=0.05, lower=0.0, upper=1.0):
        if lower >= upper:
            raise ValueError("lower must be smaller than upper")
        self.kp, self.ki, self.kd = kp, ki, kd
        self.lower, self.upper = lower, upper
        self.integral = 0.0
        self.previous = None

    def update(self, target, measurement, dt):
        if dt <= 0:
            raise ValueError("dt must be positive")
        error = target - measurement
        derivative = 0.0 if self.previous is None else -(measurement - self.previous) / dt
        proposed_integral = self.integral + error * dt
        proposed = self.kp * error + self.ki * proposed_integral + self.kd * derivative
        # If saturation would be driven further, freeze integration. It may
        # still move back toward the allowed range when the error changes sign.
        if not ((proposed > self.upper and error > 0) or (proposed < self.lower and error < 0)):
            self.integral = proposed_integral
        result = self.kp * error + self.ki * self.integral + self.kd * derivative
        self.previous = measurement
        return max(self.lower, min(self.upper, result))


def generate(mode):
    if mode == "rc":
        return [(i * .001, rc_voltage(i * .001, 10000, 1e-6)) for i in range(101)], "capacitor_voltage_v"
    if mode == "filter":
        filtered = 0.0
        rows = []
        for i in range(1000):
            t = i * .01
            raw = (1.0 if t >= 1 else 0.0) + .2 * math.sin(2 * math.pi * 18 * t)
            filtered = lowpass(filtered, raw, .01, .1)
            rows.append((t, raw, filtered))
        return rows, "raw,filtered"
    if mode == "alias":
        # 70 Hz sampled at 100 Hz appears as -30 Hz: no digital filter can
        # recover the unique original frequency from these samples alone.
        return [(i / 100, math.sin(2 * math.pi * 70 * i / 100), -math.sin(2 * math.pi * 30 * i / 100)) for i in range(101)], "signal_70hz,alias_minus30hz"
    controller, measurement, rows = PID(), 0.0, []
    for i in range(2000):
        t = i * .01
        target = .7 if t >= 1 else 0.0
        control = controller.update(target, measurement, .01)
        measurement += .01 * (control - measurement) / .5  # simulated first-order plant
        rows.append((t, target, measurement, control))
    return rows, "target,measurement,control"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=["rc", "filter", "alias", "pid", "power"])
    parser.add_argument("--output", type=Path, default=Path("data/math.csv"))
    parser.add_argument("--plot", type=Path, help="optional PNG; requires matplotlib")
    args = parser.parse_args()
    if args.mode == "power":
        value = average_current(120, .15, 2, 60)
        print(f"SIMULATED same-rail average: {value:.3f} mA; ideal 1000 mAh: {1000/value:.1f} h")
        return
    rows, names = generate(args.mode)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["time_s", *names.split(",")])
        writer.writerows(rows)
    print(f"SIMULATED data written: {args.output}")
    if args.plot:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        for index, name in enumerate(names.split(","), start=1):
            plt.plot([r[0] for r in rows], [r[index] for r in rows], label=name)
        plt.xlabel("Time (s)")
        plt.ylabel("Voltage (V)" if args.mode == "rc" else "Normalized value")
        plt.title(f"SIMULATION: {args.mode}")
        plt.legend()
        plt.grid(alpha=.25)
        plt.tight_layout()
        args.plot.parent.mkdir(parents=True, exist_ok=True)
        plt.savefig(args.plot, dpi=180)
        plt.close()


if __name__ == "__main__":
    main()
