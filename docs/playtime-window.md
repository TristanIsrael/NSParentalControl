# Playtime Window and Blocking Rules

This document defines the rules enforced by the sysmodule monitor to decide when a running game must be stopped. It is the reference for any change touching time-based play limits.

## Blocking rule

The game is stopped (timeout screen shown) when **any** of the following conditions is true:

1. The user's **daily playtime is used up** (remaining minutes == 0),
2. The **current time is before the playtime window** (too early to play),
3. The **current time is after the playtime window** (bedtime reached),
4. The **running game is blocked for the user** (see [Blocked games](#blocked-games)).

The four conditions are independent and combined with a logical OR.

## Time source (tamper protection)

All time-based decisions must use the **adjusted time**, never the raw system time.

- `core::computeAdjustedTime(currentTime, lastTime, elapsedMinutes, offset)` (`sysmodule/source/playtime_rules.cpp`), exposed to the monitor through `helpers::getAdjustedTime` (`sysmodule/source/helpers.cpp`), detects jumps of the system clock larger than `core::kTimeJumpThreshold` (5 minutes) and compensates them with a cumulative offset.
- A jump is the difference between the current time and the **expected** time: the previous observed time plus the real time elapsed since then (`elapsedMinutes`), modulo 24 hours. The real elapsed time is measured with the standard steady clock (`timeGetStandardSteadyClockTimePoint`, `helpers::getSteadyClockSeconds`), which keeps running while the console sleeps and cannot be changed by the user. Therefore:
  - a console sleep is not a jump (the system and the steady clocks both advance),
  - the change of day at midnight (23:59 -> 00:00) is not a jump,
  - a clock change made while the console sleeps is still detected.
- The first observation (no previous steady clock reading) never detects a jump. If the steady clock cannot be read, no jump is detected either.
- The observation state (`lastTime`, steady clock reading, offset) is shared by the monitoring loop and the `GetAdjustedCurrentTime` IPC command, and protected by a mutex in `Monitor`.
- The adjusted time is wrapped in the range `[0, 1440)` minutes.
- Purpose: a user who changes the system clock to escape the limits cannot, because the offset follows the jump.

The raw system time is read with `timeGetCurrentTime(TimeType_LocalSystemClock)` and converted to minutes since midnight by `getCurrentTimeInMinutesSinceMidnight()`.

## Playtime window

The window is defined per user by two settings (minutes since midnight, 0 = not set):

| Setting key       | Meaning                              | IPC commands                |
|-------------------|--------------------------------------|-----------------------------|
| `game_start_users`| Earliest allowed play time (window start) | `SetGameStartTime` / `GetGameStartTime` |
| `bedtime_users`   | Time at which play must stop (window end) | `SetBedtime` / `GetBedtime` |

A time of `0` means the bound is **disabled** (unlimited on that side).

Allowed play periods by configuration:

| Start (`S`) | End (`B`) | Allowed period                    |
|-------------|-----------|-----------------------------------|
| 0           | 0         | All day (no window restriction)   |
| 0           | B         | `[00:00, B)`                      |
| S           | 0         | `[S, 24:00)`                      |
| S < B       |           | `[S, B)`                          |
| S = B       |           | Never (empty window)              |
| S > B       |           | `[S, 24:00) ∪ [00:00, B)` (window crosses midnight) |

The `S > B` case is mandatory: a window like 20:00 to 08:00 must allow play at 22:00. Any reimplementation of the window check must keep supporting it.

## Blocked games

The administrator can block games per user from the overlay (Admin menu → `Blocked games`).

- Stored in the `blacklist` setting (STRING), serialized as `uid=titleId,titleId;uid=titleId` with decimal title IDs (`core::serializeUserTitles` / `core::deserializeUserTitles` in `playtime_rules.cpp`, which ignore invalid title IDs without throwing).
- IPC commands: `GetBlacklistedTitlesCount`, `GetBlacklistedTitle`, `AddTitleToBlacklist`, `RemoveTitleFromBlacklist`.
- The monitor passes `helpers::isTitleBlacklistedForUser(userId, titleId)` to `core::evaluatePlayRules(...)`, which blocks when it is true. The user is notified that the game is not allowed, then the game is closed after the usual countdown.
- A blocked game is detected within 5 seconds of its launch, and the block is confirmed 10 seconds later with the current user (see [Enforcement](#enforcement)).

## Daily limit

- Stored per user in `daily_limit_users` (minutes per day).
- Usage is accumulated per day in the history database (`sessions.bin`, see `docs/database-binary-format.md`).
- `remaining = daily_limit - usage_today`; when `remaining <= 0` the game is stopped.
- Notifications about the remaining time are sent every 15 minutes and every minute during the last 5 minutes (only when `show_remaining_time` is enabled and Ultrahand is used).

## Enforcement

- The rules are evaluated in `Monitor::loop()` (`sysmodule/source/monitor.cpp`), once per minute while a game is running. The iterations are scheduled on fixed deadlines (system tick), so that one counted minute is one real minute whatever the processing time.
- Between two iterations, the monitor verifies the rules every 5 seconds without recording any usage (`Monitor::enforceRulesForRunningApplication()`), so that a game launched while the user is not allowed to play (limit used up, outside the window) is stopped within a few seconds instead of up to one minute.
- Many games are launched before the user is selected: until then, the current user is the previous one. Before the countdown starts, the decision is therefore confirmed 10 seconds later with the current user (`Monitor::stopApplication`), and it is evaluated again before each countdown notification: the countdown is cancelled when the current user is allowed to play.
- No decision is taken and no minute is recorded until the current user is known (`Monitor::isUserContextSettled`):
  - while the user selection ("Who is playing?", player select library applet `0100000000001007`) is displayed (`helpers::isUserSelectionInProgress`);
  - for a title whose user is not selected by the HOME menu before its launch (NACP `StartupUserAccount` not set, `helpers::isUserSelectedBeforeLaunch`), during the first 30 seconds after the launch, until the user selection has been displayed and closed: the game may ask for the user after its loading screens. The wait ends as soon as the user is selected; the 30 seconds elapse only for a game that never asks for the user, which then uses the current user.
  - The block confirmation waits until the current user is known (up to 2 minutes), so no warning nor countdown is shown before the user is selected.
- A stopped game is closed after a 60-second countdown (a notification every 10 seconds). It is closed from the HOME menu with a virtual controller (HOME, X, A), so that the system does not report an error; if it is still running, it is terminated through `ns:dev`, then `pm:shell`.
- The monitoring loop never exits. When the service is disabled (setting `enabled` = 0, or `Monitor::stop()`), the loop goes idle and automatically resumes when the service is enabled again; a reboot is not required. While idle, the tamper-protection state is reset so that the idle period is not mistaken for a system clock manipulation.
- The pure decision logic (window membership, tamper-protected time, the block/notify decision, the notification cadence, per-user serialization) lives in `sysmodule/source/playtime_rules.{h,cpp}`, which has **no Switch dependency** so it can be unit-tested on the host (`make -C tests`, see `/tests`).
- The final block/notify decision is consolidated in `core::evaluatePlayRules(...)` (`playtime_rules.cpp`), which `Monitor::loop` calls once per minute. `core::shouldSendNotification(...)` implements the "every 15 minutes and every minute in the last 5" cadence.
- **The rules must only be enforced in the sysmodule.** The overlay is a passive client: it displays state and settings but never decides to block.

## Code map

| Concern                          | Location                                                        |
|----------------------------------|-----------------------------------------------------------------|
| Pure playtime logic (no Switch dep) | `sysmodule/source/playtime_rules.{h,cpp}` (host unit tests: `make -C tests`) |
| Rule evaluation                  | `sysmodule/source/monitor.cpp` (`Monitor::loop`)                |
| Block/notify decision            | `sysmodule/source/playtime_rules.cpp` (`evaluatePlayRules`)      |
| Notification cadence             | `sysmodule/source/playtime_rules.cpp` (`shouldSendNotification`) |
| IPC command contract (host test) | `tests/command_test.cpp` (guards `sysmodule/source/ipc/Command.hpp`) |
| Tamper-protected time            | `sysmodule/source/playtime_rules.cpp` (`computeAdjustedTime`) via `helpers.cpp` (`getAdjustedTime`) |
| Window settings storage          | `sysmodule/source/helpers.cpp` (`getGameStartTime` / `setGameStartTime`, `getBedtime` / `setBedtime`) |
| Setting keys                     | `sysmodule/source/database/settings.h`                          |
| IPC commands                     | `sysmodule/source/ipc/Command.hpp` + `service.cpp` (`commandThread`) |
| Setup UI (Playtime window panel) | `overlay/src/gui/panel_setup_limits_user.cpp`                   |
| Main menu display                | `overlay/src/gui/panel_main_menu.cpp`                           |

## Notes for future changes

- Per-user values are serialized as `uid=value;uid=value` strings inside a single STRING setting; a value of `0` means disabled.
- The overlay and the sysmodule share a duplicated `Command` enum (`overlay/src/Command.hpp` and `sysmodule/source/ipc/Command.hpp`). Both files must stay identical when adding a command.
- Changing the serialized format or the settings keys breaks existing users' data: update `docs/database-binary-format.md` and bump the version constant if you do.
