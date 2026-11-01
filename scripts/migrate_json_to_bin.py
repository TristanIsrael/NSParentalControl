#!/usr/bin/env python3
"""
NS Parental Control - JSON to binary database migration

Converts the data files of the versions prior to 2.0 (settings.json and
sessions.json) into the binary files used since 2.0 (settings.bin and
sessions.bin), so that upgrading does not lose the settings and the history.
The binary format is documented in docs/database-binary-format.md.

Mapping of the settings:
  - admin_pin          decoded from the 1.x obfuscation, stored as "b1,b2,b3,b4"
  - daily_limit_users  JSON object {"uid": minutes} -> "uid=minutes;uid=minutes"
  - enabled, log_level, show_remaining_time, working_mode, authentication
                       copied as integers
  - daily_limit_global, daily_limit_game
                       not used since 2.0, dropped

Usage:
    python3 migrate_json_to_bin.py [options]

Options:
    --settings-in FILE    Path to settings.json (default: ./settings.json)
    --history-in FILE     Path to sessions.json (default: ./sessions.json)
    --settings-out FILE   Path to settings.bin (default: ./settings.bin)
    --history-out FILE    Path to sessions.bin (default: ./sessions.bin)
    --help                Show this help message and exit
"""

import argparse
import json
import struct
import sys

HISTORY_MAGIC = 0x4E535043  # "NSPC"
HISTORY_VERSION = 1
SETTINGS_MAGIC = 0x4E535053  # "NSPS"
SETTINGS_VERSION = 1

TYPE_INTEGER = 0
TYPE_DOUBLE = 1
TYPE_STRING = 2

INTEGER_KEYS = ("enabled", "log_level", "show_remaining_time", "working_mode", "authentication")
DROPPED_KEYS = ("daily_limit_global", "daily_limit_game")


def decode_v1_value(value):
    """Reverses the 1.x obfuscation of encrypted strings (byte swap then XOR 0x31)."""
    data = bytearray(value.encode("latin-1"))
    for i in range(0, len(data) - 3, 4):
        data[i], data[i + 2] = data[i + 2], data[i]
        data[i + 1], data[i + 3] = data[i + 3], data[i + 1]
    return bytes(b ^ 0x31 for b in data).decode("ascii")


def serialize_user_values(values):
    """Serializes per-user values as "uid=value;uid=value" (sorted like a std::map)."""
    return ";".join(f"{uid}={int(values[uid])}" for uid in sorted(values))


def convert_settings(path):
    """Returns the binary settings as (key, type, value, encrypted) tuples sorted by key."""
    with open(path, encoding="utf-8") as f:
        settings = json.load(f)["settings"]

    converted = {}
    for setting in settings:
        key = setting["key"]
        if key in DROPPED_KEYS:
            print(f"  {key}: dropped (not used since 2.0)")
        elif key == "admin_pin":
            pin = decode_v1_value(setting["value"]) if setting.get("encrypted") else setting["value"]
            converted[key] = (TYPE_STRING, pin, True)
            print(f"  {key}: converted")
        elif key == "daily_limit_users":
            limits = json.loads(setting["value"]) if setting["value"] else {}
            converted[key] = (TYPE_STRING, serialize_user_values(limits), False)
            print(f"  {key}: {len(limits)} user(s)")
        elif key in INTEGER_KEYS:
            converted[key] = (TYPE_INTEGER, int(setting["value"]), False)
            print(f"  {key}: {setting['value']}")
        else:
            print(f"  {key}: unknown key, dropped")

    return [(key,) + converted[key] for key in sorted(converted)]


def write_settings(path, settings):
    data = bytearray(struct.pack("<IIQ", SETTINGS_MAGIC, SETTINGS_VERSION, len(settings)))
    for key, setting_type, value, encrypted in settings:
        key_bytes = key.encode("ascii")
        data += struct.pack("<II", setting_type, len(key_bytes)) + key_bytes
        if setting_type == TYPE_INTEGER:
            data += struct.pack("<Q", value)
        else:
            value_bytes = value.encode("ascii")
            data += struct.pack("<I", len(value_bytes)) + value_bytes + struct.pack("<B", 1 if encrypted else 0)
    with open(path, "wb") as f:
        f.write(data)


def convert_history(path):
    """Returns the history entries as (uid_low, uid_high, date, title_id, duration) tuples."""
    with open(path, encoding="utf-8") as f:
        history = json.load(f)["history"]

    entries = []
    for entry in history:
        uid_low, uid_high = (int(part) for part in entry["uid"].split(":"))
        entries.append((uid_low, uid_high, entry["date"], int(entry["title_id"]), int(entry["duration_in_min"])))
    return entries


def write_history(path, entries):
    data = bytearray(struct.pack("<IIQ", HISTORY_MAGIC, HISTORY_VERSION, len(entries)))
    for uid_low, uid_high, date, title_id, duration in entries:
        date_bytes = date.encode("ascii")
        data += struct.pack("<QQIQH", uid_low, uid_high, len(date_bytes), title_id, duration) + date_bytes
    with open(path, "wb") as f:
        f.write(data)


def main():
    parser = argparse.ArgumentParser(description="Converts the 1.x JSON data files into the 2.x binary files.")
    parser.add_argument("--settings-in", default="settings.json")
    parser.add_argument("--history-in", default="sessions.json")
    parser.add_argument("--settings-out", default="settings.bin")
    parser.add_argument("--history-out", default="sessions.bin")
    args = parser.parse_args()

    try:
        print(f"Settings: {args.settings_in} -> {args.settings_out}")
        write_settings(args.settings_out, convert_settings(args.settings_in))

        print(f"History: {args.history_in} -> {args.history_out}")
        entries = convert_history(args.history_in)
        write_history(args.history_out, entries)
        print(f"  {len(entries)} entrie(s)")
    except (OSError, KeyError, ValueError) as error:
        print(f"Error: {error}")
        sys.exit(1)


if __name__ == "__main__":
    main()
