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

## Translations (i18n)

Only the strings that reach the user are translated: the on-screen strings (overlay) and the notification strings (sysmodule). English and French are supported; the language is the console language, read once at startup with the SET service (`setGetSystemLanguage` + `setMakeLanguage`, French including Canadian French, everything else falls back to English).

- Each component has its own module in its own `i18n/` directory — `sysmodule/source/i18n/i18n.{h,cpp}` and `overlay/src/i18n/i18n.{h,cpp}` — exposing `alefbet::pctrl::i18n::{Language, tr, trf, current, setLanguage, detectSystemLanguage}`. The two modules are independent (same convention as the duplicated IPC enum).
- Each non-source language has its own translation table file in the same `i18n/` directory (`i18n_fr.cpp`, ...); English is the source language and has no table — the English strings live in the call sites and `tr()` falls back to them.
- Every on-screen string (overlay) and every notification string (sysmodule) must go through `tr("English source text")` / `trf("English template", value)`; do not hardcode user-facing literals. The lookup is done on the English source string, and a missing translation falls back to the source text.
- The SET-service call is isolated in `detectSystemLanguage()` behind `#ifdef __SWITCH__`, so the module stays host-compilable and covered by `tests/i18n_test.cpp`.
- The authoritative description is `docs/translations.md` — keep it in sync when adding strings or languages.

## Data storage

- Settings (`settings.bin`) and history (`sessions.bin`) are stored in binary files under `/config/parental_control/`. Format specification: `docs/database-binary-format.md`.
- Per-user values are serialized as `uid=value;uid=value` strings inside a single STRING setting key (`daily_limit_users`, `bedtime_users`, `game_start_users`). A value of `0` means "not set / disabled".
- Time bounds are minutes since midnight (0-1439).
- Do not change the binary format, the serialized format, or the setting keys without updating `docs/database-binary-format.md` and bumping the version constant. Existing users' data must not be silently lost.

## Build

### Toolchain / build environment

- The Switch targets require the devkitPro **devkitA64** toolchain and **libnx** (Switch homebrew SDK); they cannot be built without them.
- Install the environment with `bash scripts/setup-toolchain.sh` (Debian/Ubuntu). It follows the [official devkitPro_pacman procedure](https://devkitpro.org/wiki/devkitPro_pacman): installs `dkp-pacman`, refreshes the package DB, and installs the `switch-dev` group (devkitA64 + libnx).
- After installation, export the variables the script prints (typically `DEVKITPRO=/opt/devkitpro`, plus `DEVKITARM`, `DEVKITPPC`, and `/opt/devkitpro/bin` in `PATH`).
- Known network limitation: `devkitpro.org` is behind Cloudflare and blocks datacenter / cloud egress IPs (HTTP 403 regardless of User-Agent). If the script aborts at the reachability check, run it from a normal (residential/office) connection.
- Full build-environment documentation lives in `Readme.md` → *Build and install*.

### Build commands

- Host unit tests (no Switch SDK required): `make -C tests` builds and runs the host test suites — the pure playtime rules (`tests/playtime_rules_test.cpp`), the IPC command contract (`tests/command_test.cpp`), and the translation module (`tests/i18n_test.cpp`).
- Toolchain check (requires the devkitPro toolchain, not run on the host): `make -C tests switch` cross-compiles `tests/switch_only_test.cpp` for the Switch target.
- Sysmodule: `make` in `sysmodule/` → `out.nosync/exefs.nsp`.
- Overlay: `make` in `overlay/` → `parental_control.ovl`.
- Both Makefiles carry an `APP_VERSION`; when a feature spans both components, bump both to the same value and update `sysmodule/toolbox.json` (`version` field).
- Versioning rule (scheme `MAJOR.MINOR.PATCH`, e.g. `2.2.0`):
  - A bug fix bumps the last digit (patch) only.
  - A new feature bumps the second digit (minor) and resets the last digit to 0.
  - The first digit (major) is bumped only by the product maintainer.

## Coding standards

- Use C++-20 for all new files
- Follow the existing naming conventions
- New source files must keep the GPLv3 license header used by the other files
- User-facing strings (on-screen in the overlay, notifications in the sysmodule) must be translated through the i18n modules (see the Translations section and `docs/translations.md`)
- Log with `logDebug` / `logInfo` / `logError` using the `[Component] ` prefix convention (e.g. `[Monitor]`, `[Service]`, `[IPC]`)
- The pure playtime logic (no Switch dependency) lives in `sysmodule/source/playtime_rules.{h,cpp}` and is unit-tested on the host: `make -C tests`. It covers the window check, tamper-protected time, the `evaluatePlayRules` block/notify decision, the `shouldSendNotification` cadence, per-user serialization, and (in `tests/command_test.cpp`) the IPC command contract. Keep the module free of any Switch *runtime* dependency (the libnx base types from `<switch/types.h>` are allowed, as they are plain typedefs), and add or update the matching tests in `/tests` when changing the play rules. Tests that require the toolchain/console go in `tests/switch_only_test.cpp` (built via `make -C tests switch`).
- Verify remaining changes by building both components

## Chat rules

- Respond in the same language as the prompt's
- Never commit or push unless explicitly asked to do so

## Documentation standards

- Write documentation, comments and commits in english language even if the prompt is in another language.
- Update the documentation in /docs and the Doxygen comments of the source code up to date when applying changes.
- The Doxygen documentation must be written only in the implementation files unless the function is encoded in the header file.
- Keep `Readme.md` ("Current features") and `docs/manual.md` in sync with user-visible changes.
- When a rule changes (blocking rules, storage format, IPC contract), update the matching doc in `/docs` in the same change.
