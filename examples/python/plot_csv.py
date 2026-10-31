"""Plot a column from serial_tool CSV. Missing readings stay missing."""
import argparse
import csv
from datetime import datetime
import math
from pathlib import Path


def load_samples(path, field):
    points = []
    with Path(path).open(encoding="utf-8", newline="") as stream:
        reader = csv.DictReader(stream)
        if not reader.fieldnames or field not in reader.fieldnames or "received_utc" not in reader.fieldnames:
            raise ValueError("CSV needs received_utc and the selected field")
        origin = None
        for row in reader:
            instant = datetime.fromisoformat(row["received_utc"])
            if origin is None:
                origin = instant
            value = float(row[field]) if row[field] and row.get("valid", "").lower() != "false" else math.nan
            points.append(((instant - origin).total_seconds(), value))
    return points


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--field", default="temperature_c")
    parser.add_argument("--output", type=Path, default=Path("data/telemetry.png"))
    parser.add_argument("--watch", action="store_true", help="re-read a growing CSV every second")
    args = parser.parse_args()
    import matplotlib
    if not args.watch:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, axis = plt.subplots()

    def draw(_=None):
        try:
            points = load_samples(args.csv, args.field)
        except (ValueError, OSError):
            if args.watch:
                return
            raise
        axis.clear()
        axis.plot([p[0] for p in points], [p[1] for p in points], marker=".")
        axis.set(xlabel="Elapsed host time (s)", ylabel=args.field, title="ESP32 telemetry (host receipt timestamps)")
        axis.grid(alpha=.25)
        fig.tight_layout()
    if args.watch:
        from matplotlib.animation import FuncAnimation
        animation = FuncAnimation(fig, draw, interval=1000, cache_frame_data=False)
        plt.show()
    else:
        draw()
        args.output.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(args.output, dpi=180)
        plt.close(fig)
        print(args.output)


if __name__ == "__main__":
    main()
