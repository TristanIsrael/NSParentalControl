# Playtime Window and Blocking Rules

This document defines the rules enforced by the sysmodule monitor to decide when a running game must be stopped. It is the reference for any change touching time-based play limits.

## Blocking rule

The game is stopped (timeout screen shown) when **any** of the following conditions is true:

1. The user's **daily playtime is used up** (remaining minutes == 0),
2. The **current time is before the playtime window** (too early to play),
3. The **current time is after the playtime window** (bedtime reached).

The three conditions are independent and combined with a logical OR.

## Time source (tamper protection)

All time-based decisions must use the **adjusted time**, never the raw system time.

- `helpers::getAdjustedTime(currTime, lastTime, offset)` (`sysmodule/source/helpers.cpp`) detects jumps of the system clock larger than `BEDTIME_TIME_JUMP_THRESHOLD` (5 minutes) between two monitor iterations and compensates them with a cumulative offset.
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

## Daily limit

- Stored per user in `daily_limit_users` (minutes per day).
- Usage is accumulated per day in the history database (`sessions.bin`, see `docs/database-binary-format.md`).
- `remaining = daily_limit - usage_today`; when `remaining <= 0` the game is stopped.
- Notifications about the remaining time are sent every 15 minutes and every minute during the last 5 minutes (only when `show_remaining_time` is enabled and Ultrahand is used).

## Enforcement

- The rules are evaluated in `Monitor::loop()` (`sysmodule/source/monitor.cpp`), once per minute while a game is running.
- **The rules must only be enforced in the sysmodule.** The overlay is a passive client: it displays state and settings but never decides to block.

## Code map

| Concern                          | Location                                                        |
|----------------------------------|-----------------------------------------------------------------|
| Rule evaluation                  | `sysmodule/source/monitor.cpp` (`Monitor::loop`)                |
| Tamper-protected time            | `sysmodule/source/helpers.cpp` (`getAdjustedTime`)              |
| Window settings storage          | `sysmodule/source/helpers.cpp` (`getGameStartTime` / `setGameStartTime`, `getBedtime` / `setBedtime`) |
| Setting keys                     | `sysmodule/source/database/settings.h`                          |
| IPC commands                     | `sysmodule/source/ipc/Command.hpp` + `service.cpp` (`commandThread`) |
| Setup UI (Playtime window panel) | `overlay/src/gui/panel_setup_limits_user.cpp`                   |
| Main menu display                | `overlay/src/gui/panel_main_menu.cpp`                           |

## Notes for future changes

- Per-user values are serialized as `uid=value;uid=value` strings inside a single STRING setting; a value of `0` means disabled.
- The overlay and the sysmodule share a duplicated `Command` enum (`overlay/src/Command.hpp` and `sysmodule/source/ipc/Command.hpp`). Both files must stay identical when adding a command.
- Changing the serialized format or the settings keys breaks existing users' data: update `docs/database-binary-format.md` and bump the version constant if you do.
