# Binary Database Format

## Overview

Parental control data is persisted in binary files on the SD card. This replaces the previous JSON-based storage for better performance and to eliminate external library dependencies.

**Common path prefix:** `/config/parental_control/`

## Design Rationale

The binary format was chosen over JSON for the following reasons:

1. **Performance**: Single read/write operation, no parsing overhead
2. **Size**: No textual serialization overhead (no keys, quotes, or whitespace)
3. **Simplicity**: No external library dependencies (eliminates nlohmann/json and mbedtls)
4. **Reliability**: No hash verification overhead

## In-Memory Strategy (Both Modules)

- **Cache**: Data is cached in memory for the application lifetime.
- **Synchronization flag**: A `*_synchronized_` flag indicates whether the cache matches the file on disk.
- **Lazy loading**: Data is read from disk only if not synchronized, triggered by the first access.
- **Write-on-change**: Data is written after every modification.
- **Thread safety**: All read/write operations are protected by a module-specific mutex.

---

## Session History (`sessions.bin`)

**File:** `sessions.bin`

### In-Memory Strategy

- Cache: `History history_`
- Sync flag: `data_synchronized_`
- Mutex: `mutex_database_`, held by `getHistory()` and `addToHistory()` for the whole operation (load, read or update, save)
- Trigger: `loadDatabase()` called by `getHistory()` and by `addToHistory()`. The history is always loaded before it is updated: otherwise, after a reboot, the usage already recorded that day would be ignored and the file would be overwritten with the new entry only. If the history cannot be loaded, the usage is not recorded.
- Unreadable file: when `sessions.bin` cannot be parsed (bad magic, unsupported version, truncated entry), it is moved to `sessions.corrupted.bin` (kept for analysis) and a new history is started, so that the usage recording is not blocked.

### Uniqueness Rules

Entries are identified by the triplet `(uid, date, titleId)`. When an entry with the same triplet already exists, the new duration is **merged** into the existing entry rather than creating a new row.

### Binary Format

#### Header (16 bytes)

| Offset | Size | Field      | Description                      |
|--------|------|------------|----------------------------------|
| 0      | 4    | magic      | `0x4E535043` ("NSPC")            |
| 4      | 4    | version    | Format version (currently `1`)   |
| 8      | 8    | entryCount | Number of entries (u64, LE)      |

#### Entry (variable size = 30 + dateLength)

| Offset | Size   | Field             | Description                              |
|--------|--------|-------------------|------------------------------------------|
| 0      | 8      | uidLow            | `uid.uid[0]`                             |
| 8      | 8      | uidHigh           | `uid.uid[1]`                             |
| 16     | 4      | dateLength        | Length of date string (u32)              |
| 20     | 8      | titleId           | Title identifier (u64)                   |
| 28     | 2      | durationInMinutes | Session duration in minutes (u16)        |
| 30     | N      | date              | ASCII string, format `YYYYMMDD`          |

#### Total entry size

```
sizeof(BinEntry) + dateLength == 30 + dateLength
```

For the standard `YYYYMMDD` format, `dateLength` is 10, so each entry is 40 bytes.

---

## Settings (`settings.bin`)

**File:** `settings.bin`

### In-Memory Strategy

- Cache: `Settings settings_` (std::map of key-value pairs)
- Sync flag: `settings_synchronized_`
- Mutex: `mutex_settings_`
- Trigger: `loadSettings()` called on first access

### Binary Format

#### Header (16 bytes)

| Offset | Size | Field      | Description                      |
|--------|------|------------|----------------------------------|
| 0      | 4    | magic      | `0x4E535053` ("NSPS")            |
| 4      | 4    | version    | Format version (currently `1`)   |
| 8      | 8    | entryCount | Number of settings (u64, LE)     |

#### Entry Header (8 bytes, fixed)

| Offset | Size | Field      | Description                    |
|--------|------|------------|--------------------------------|
| 0      | 4    | type       | SettingType enum (0=INT, 1=DOUBLE, 2=STRING) |
| 4      | 4    | keyLength  | Length of key string (u32)     |

#### Entry Payload (variable, depends on type)

After the entry header and key string, the payload varies by type:

**INTEGER (`type == 0`):**

| Offset | Size | Field      | Description                  |
|--------|------|------------|------------------------------|
| 0      | 8    | int_value  | Integer value (u64)          |

Total: 8 bytes after key.

**DOUBLE (`type == 1`):**

| Offset | Size | Field       | Description                     |
|--------|------|-------------|---------------------------------|
| 0      | 8    | double_bits | Raw bit representation of double (u64, via `std::bit_cast`) |

Total: 8 bytes after key.

**STRING (`type == 2`):**

| Offset | Size      | Field        | Description                     |
|--------|-----------|--------------|---------------------------------|
| 0      | 4         | valueLength  | Length of value string (u32)    |
| 4      | N         | value        | ASCII value string              |
| 4+N    | 1         | encrypted    | `1` if encrypted, `0` otherwise |

Total: `5 + valueLength` bytes after key.

#### Total entry size

```
8 (entry header) + keyLength + payloadSize
```

Where `payloadSize` is `8` for INTEGER/DOUBLE, or `5 + valueLength` for STRING.

### Known Setting Keys

Defined in `settings.h`:

| Key                 | Type   | Description                                                          |
|---------------------|--------|----------------------------------------------------------------------|
| `daily_limit_users` | STRING | Per-user daily limit in minutes (serialized per-user value)          |
| `bedtime_users`     | STRING | Per-user bed time, end of the playtime window (serialized per-user value) |
| `game_start_users`  | STRING | Per-user earliest allowed play time, start of the playtime window (serialized per-user value) |
| `admin_pin`         | STRING | Admin PIN (may be encrypted)                                         |
| `working_mode`      | INTEGER| 0=Info, 1=Blocking                                                   |
| `show_remaining_time` | INTEGER | Display flag (0 or 1)                                              |
| `enabled`           | INTEGER| Master enable flag (0 or 1)                                          |
| `log_level`         | INTEGER| Logging verbosity level                                              |
| `blacklist`         | STRING | Blacklisted title IDs                                                |
| `authentication`    | INTEGER| Authentication mode                                                  |

### Per-user values

The keys ending in `_users` share a common serialization format: a single STRING setting holds one entry per user, formatted as `uid=value;uid=value`.

- `uid` is the user UID as a hex string (`accountUidToString`).
- `value` depends on the key:
  - `daily_limit_users`: daily limit in minutes per day.
  - `bedtime_users` / `game_start_users`: time in minutes since midnight (0-1439).
- A value of `0` means the bound is not set (disabled).

See `docs/playtime-window.md` for the rules applied to the playtime window bounds.

---

## Migration from the 1.x JSON files

The versions prior to 2.0 stored the data in `settings.json` and `sessions.json`. The sysmodule does not read these files, so the settings (PIN, limits) and the history are reset when upgrading. The script `scripts/migrate_json_to_bin.py` converts them into `settings.bin` and `sessions.bin` on a computer:

```
python3 scripts/migrate_json_to_bin.py --settings-in settings.json --history-in sessions.json
```

- `admin_pin` is decoded from the 1.x obfuscation and stored as `b1,b2,b3,b4`.
- `daily_limit_users` (a JSON object `{"uid": minutes}` in 1.3) is serialized as `uid=minutes;uid=minutes`.
- `enabled`, `log_level`, `show_remaining_time`, `working_mode` and `authentication` are copied as integers.
- `daily_limit_global` and `daily_limit_game` are not used since 2.0 and are dropped.

Copy the generated files to `/config/parental_control/` while the sysmodule is not running the new version yet (before rebooting after the upgrade), then check them with `scripts/viewer.py --both`.

---

## Versioning (Both Modules)

Each module has its own `DB_VERSION` or `SETTINGS_VERSION` constant, incremented when the file format changes. A version mismatch at load time causes the file to be treated as invalid and replaced with defaults.
