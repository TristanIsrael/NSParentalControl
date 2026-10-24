#!/usr/bin/env python3
"""
NS Parental Control - Binary Database Viewer

Displays the contents of sessions.bin (history) and settings.bin (settings)
in a human-readable format based on the documented binary format.

Usage:
    python3 viewer.py [options]

Options:
    --history FILE    Path to sessions.bin (default: ./sessions.bin)
    --settings FILE   Path to settings.bin (default: ./settings.bin)
    --help            Show this help message and exit
"""

import argparse
import struct
import sys


def read_history(path):
    """Read and display history database contents."""
    try:
        with open(path, "rb") as f:
            data = f.read()
    except FileNotFoundError:
        print(f"Error: file not found: {path}")
        sys.exit(1)

    if len(data) < 16:
        print("Error: file too small for header")
        sys.exit(1)

    magic, version, entry_count = struct.unpack("<IIQ", data[:16])
    magic_str = struct.pack("<I", magic).decode("ascii", errors="replace")

    print(f"History Database: {path}")
    print(f"  Magic: {magic} ({magic_str})")
    print(f"  Version: {version}")
    print(f"  Entries: {entry_count}")
    print(f"  Size: {len(data)} bytes")
    print()

    if entry_count == 0:
        print("  (no entries)")
        return

    print(f"  {'UID':>32s}  {'Date':>10s}  {'Title ID':>16s}  {'Duration (min)':>15s}")
    print(f"  {'-'*32}  {'-'*10}  {'-'*16}  {'-'*15}")

    offset = 16
    for i in range(entry_count):
        if offset + 30 > len(data):
            print(f"  Warning: truncated entry at index {i}")
            break

        uid_low, uid_high, date_length, title_id, duration = struct.unpack(
            "<QQIQH", data[offset:offset + 30]
        )
        offset += 28

        if offset + date_length > len(data):
            print(f"  Warning: truncated date string at entry {i}")
            break

        date_str = data[offset:offset + date_length].decode("ascii", errors="replace")
        offset += date_length

        uid_str = f"{uid_low}:{uid_high}"
        title_hex = f"{title_id:016X}"

        print(f"  {uid_str:>32s}  {date_str:>10s}  {title_hex:>16s}  {duration:>15d}")


def read_settings(path):
    """Read and display settings database contents."""
    try:
        with open(path, "rb") as f:
            data = f.read()
    except FileNotFoundError:
        print(f"Error: file not found: {path}")
        sys.exit(1)

    if len(data) < 16:
        print("Error: file too small for header")
        sys.exit(1)

    magic, version, entry_count = struct.unpack("<IIIQ", data[:16])
    magic_str = struct.pack("<I", magic).decode("ascii", errors="replace")

    print(f"Settings Database: {path}")
    print(f"  Magic: {magic} ({magic_str})")
    print(f"  Version: {version}")
    print(f"  Settings: {entry_count}")
    print(f"  Size: {len(data)} bytes")
    print()

    if entry_count == 0:
        print("  (no settings)")
        return

    type_names = {0: "INTEGER", 1: "DOUBLE", 2: "STRING"}

    print(f"  {'Key':<40s}  {'Type':<10s}  {'Value':>20s}")
    print(f"  {'-'*40}  {'-'*10}  {'-'*20}")

    offset = 16
    for i in range(entry_count):
        if offset + 8 > len(data):
            print(f"  Warning: truncated entry header at index {i}")
            break

        entry_type, key_length = struct.unpack("<II", data[offset:offset + 8])
        offset += 8

        if offset + key_length > len(data):
            print(f"  Warning: truncated key string at setting {i}")
            break

        key = data[offset:offset + key_length].decode("ascii", errors="replace")
        offset += key_length

        type_name = type_names.get(entry_type, f"UNKNOWN({entry_type})")

        if entry_type == 0:
            if offset + 8 > len(data):
                print(f"  Warning: truncated value for '{key}'")
                break
            int_val, = struct.unpack("<Q", data[offset:offset + 8])
            offset += 8
            value_str = str(int_val)

        elif entry_type == 1:
            if offset + 8 > len(data):
                print(f"  Warning: truncated value for '{key}'")
                break
            double_bits, = struct.unpack("<Q", data[offset:offset + 8])
            offset += 8
            double_val = struct.unpack("<d", struct.pack("<Q", double_bits))[0]
            value_str = f"{double_val:.4f}" if double_val != 0 else "0.0000"

        elif entry_type == 2:
            if offset + 4 > len(data):
                print(f"  Warning: truncated value length for '{key}'")
                break
            value_length, = struct.unpack("<I", data[offset:offset + 4])
            offset += 4

            if offset + value_length > len(data):
                print(f"  Warning: truncated value string for '{key}'")
                break

            value_bytes = data[offset:offset + value_length]
            value_str = value_bytes.decode("utf-8", errors="replace")
            offset += value_length

            if offset + 1 > len(data):
                print(f"  Warning: truncated encrypted flag for '{key}'")
                break

            encrypted, = struct.unpack("B", data[offset:offset + 1])
            offset += 1

            if encrypted:
                value_str = f"*** ENCRYPTED: {value_str} ***"
        else:
            value_str = "?"

        print(f"  {key:<40s}  {type_name:<10s}  {value_str:>20s}")


def main():
    parser = argparse.ArgumentParser(
        description="Display contents of NS Parental Control binary database files"
    )
    parser.add_argument(
        "--history",
        metavar="FILE",
        default="./sessions.bin",
        help="Path to sessions.bin (default: ./sessions.bin)",
    )
    parser.add_argument(
        "--settings",
        metavar="FILE",
        default="./settings.bin",
        help="Path to settings.bin (default: ./settings.bin)",
    )
    parser.add_argument(
        "--both",
        action="store_true",
        help="Display both history and settings databases",
    )

    args = parser.parse_args()

    if args.both:
        read_history(args.history)
        print()
        print("=" * 60)
        print()
        read_settings(args.settings)
    else:
        if args.history != "./sessions.bin":
            read_history(args.history)
        elif args.settings != "./settings.bin":
            read_settings(args.settings)
        else:
            print("No file specified. Use --history, --settings, or --both")
            parser.print_help()
            sys.exit(1)


if __name__ == "__main__":
    main()
