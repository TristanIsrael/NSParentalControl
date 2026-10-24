# Binary Database Format

## Overview

The history database stores session records for parental control features. Data is persisted in a single binary file on the SD card, replacing the previous JSON format for better performance and simplicity.

**File path:** `/config/parental_control/sessions.bin`

## Design Rationale

The binary format was chosen over JSON for the following reasons:

1. **Performance**: Single read/write operation, no JSON parsing overhead
2. **Size**: No textual serialization overhead (no keys, quotes, or whitespace)
3. **Simplicity**: No external library dependencies (eliminates nlohmann/json and mbedtls)
4. **Reliability**: No hash verification overhead

## In-Memory Strategy

- **Cache**: Entries are cached in memory (`History history_`) for the application lifetime.
- **Synchronization flag**: `data_synchronized_` indicates whether the cache matches the file on disk.
- **Lazy loading**: `loadDatabase()` reads from disk only if `data_synchronized_ == false`. This is triggered automatically by `getHistory()`.
- **Write-on-change**: `saveDatabase()` is called after every addition via `addToHistory()`.
- **Thread safety**: All read/write operations are protected by `mutex_database_`.

## Uniqueness Rules

Entries are identified by the triplet `(uid, date, titleId)`. When an entry with the same triplet already exists, the new duration is **merged** into the existing entry rather than creating a new row. This keeps the file compact.

## Versioning

`DB_VERSION` is incremented whenever the file format changes. A version mismatch at load time marks the file as corrupted (`isTampered_ = true`).

## Binary File Format

### Header (16 bytes)

| Offset | Size | Field      | Description                      |
|--------|------|------------|----------------------------------|
| 0      | 4    | magic      | `0x4E535043` ("NSPC")            |
| 4      | 4    | version    | Format version (currently `1`)   |
| 8      | 8    | entryCount | Number of entries (u64, LE)      |

### Entry (variable size = 30 + dateLength)

| Offset | Size   | Field             | Description                              |
|--------|--------|-------------------|------------------------------------------|
| 0      | 8      | uidLow            | `uid.uid[0]`                             |
| 8      | 8      | uidHigh           | `uid.uid[1]`                             |
| 16     | 4      | dateLength        | Length of date string (u32)              |
| 20     | 8      | titleId           | Title identifier (u64)                   |
| 28     | 2      | durationInMinutes | Session duration in minutes (u16)        |
| 30     | N      | date              | ASCII string, format `YYYYMMDD`          |

### Total entry size

```
sizeof(BinEntry) + dateLength == 30 + dateLength
```

For the standard `YYYYMMDD` format, `dateLength` is 10, so each entry is 40 bytes.
