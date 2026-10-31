"""Newline-delimited telemetry contract shared by desktop tools (Python 3.10+)."""
import json
import math

MAX_LINE_BYTES = 4096


def parse_record(line: bytes | str) -> dict:
    """Reject oversized, non-object, non-finite and incorrectly typed records.

    Unknown fields are kept so that firmware can evolve without breaking logging.
    A missing sensor is represented by valid=false and null readings, never zero.
    """
    raw = line if isinstance(line, bytes) else line.encode("utf-8")
    if len(raw) > MAX_LINE_BYTES:
        raise ValueError("record exceeds 4096 bytes")
    try:
        record = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ValueError("not a UTF-8 JSON record") from exc
    if not isinstance(record, dict):
        raise ValueError("record must be a JSON object")
    if "valid" in record and not isinstance(record["valid"], bool):
        raise ValueError("valid must be boolean")
    for key in ("temperature_c", "humidity_pct", "value"):
        value = record.get(key)
        if value is not None and (isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value)):
            raise ValueError(f"{key} must be finite numeric data or null")
    for key in ("seq", "uptime_ms"):
        value = record.get(key)
        if value is not None and (isinstance(value, bool) or not isinstance(value, int) or value < 0):
            raise ValueError(f"{key} must be a non-negative integer")
    return record
