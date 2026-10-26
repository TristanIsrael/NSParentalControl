# Project rules

This is a C++ application for Nintendo Switch running on Atmosphère with:

- The Sysmodule backend in `/sysmodule`
- The Tesla Overlay in `/overlay`
- The documentation is in `/docs`

## Scope

NS Parental Control is an offline parental control system for the Nintendo Switch. It limits children's play time without any Internet access or smartphone.

- **Gamers**: the users whose play time is limited.
- **Administrator**: the user who defines the rules, protected by a PIN (default: `A A A A`).

The project contains only two shipped components (sysmodule + overlay). Do not add new top-level components without explicit request.

## Architecture

The two components communicate through an IPC service named `pctrl`:

1. **Sysmodule** (`/sysmodule/source`) — resident backend. It monitors game usage in a 1-minute loop (`monitor.cpp`), enforces the play rules, persists data on the SD card, and exposes the IPC commands.
2. **Overlay** (`/overlay/src`) — Tesla homebrew overlay. UI for information and setup. It talks to the sysmodule **only** through IPC.

Hard rules:

- The overlay must never read or write the database files directly; all data flows through IPC commands.
- The IPC command list is duplicated in `sysmodule/source/ipc/Command.hpp` and `overlay/src/Command.hpp`. **These two enums must always stay byte-for-byte identical** (same order, same values). When adding a command: edit both files, add the handler declaration in `sysmodule/source/service.h`, its implementation and its `commandThread` case in `service.cpp`, and the client helper in `overlay/src/helpers/ipc_helpers.{h,cpp}`.
- Play rules are enforced **only** in the sysmodule (`Monitor::loop`). The overlay displays state, it never blocks.
- Do not modify anything in `sysmodule/backup/` and `sysmodule/deprecated/` (archived code).

## Blocking rules (summary)

A running game is stopped when **any** of the following is true:

1. The user's daily playtime is used up (remaining minutes == 0).
2. The adjusted current time is before the playtime window (`game_start_users`).
3. The adjusted current time is after the playtime window (`bedtime_users`).

Time comparisons must always use the tamper-protected time (`helpers::getAdjustedTime`, which compensates system clock jumps), never the raw system clock. The playtime window must keep supporting windows that cross midnight (start > end). The authoritative description of these rules is `docs/playtime-window.md` — keep it in sync when changing the logic.

## Data storage

- Settings (`settings.bin`) and history (`sessions.bin`) are stored in binary files under `/config/parental_control/`. Format specification: `docs/database-binary-format.md`.
- Per-user values are serialized as `uid=value;uid=value` strings inside a single STRING setting key (`daily_limit_users`, `bedtime_users`, `game_start_users`). A value of `0` means "not set / disabled".
- Time bounds are minutes since midnight (0-1439).
- Do not change the binary format, the serialized format, or the setting keys without updating `docs/database-binary-format.md` and bumping the version constant. Existing users' data must not be silently lost.

## Build

- Requires `devkitPro` with the Switch SDK (`export DEVKITPRO=<path>`); building on the host is not possible.
- Sysmodule: `make` in `sysmodule/` → `out.nosync/exefs.nsp`.
- Overlay: `make` in `overlay/` → `parental_control.ovl`.
- Both Makefiles carry an `APP_VERSION`; when a feature spans both components, bump both to the same value and update `sysmodule/toolbox.json` (`version` field).

## Coding standards

- Use C++-21 for all new files
- Follow the existing naming conventions
- New source files must keep the GPLv3 license header used by the other files
- Log with `logDebug` / `logInfo` / `logError` using the `[Component] ` prefix convention (e.g. `[Monitor]`, `[Service]`, `[IPC]`)
- There is no unit test suite; verify changes by building both components

## Chat rules

- Respond in the same language as the prompt's

## Documentation standards

- Write documentation, comments and commits in english language even if the prompt is in another language.
- Update the documentation in /docs and the Doxygen comments of the source code up to date when applying changes.
- The Doxygen documentation must be written only in the implementation files unless the function is encoded in the header file.
- Keep `Readme.md` ("Current features") and `docs/manual.md` in sync with user-visible changes.
- When a rule changes (blocking rules, storage format, IPC contract), update the matching doc in `/docs` in the same change.
