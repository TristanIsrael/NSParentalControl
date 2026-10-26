# Test Plan — Playtime Window (v2.2)

Complete test plan for the playtime window feature (earliest allowed play time + bed time) and regression of the existing limit features.

The reference for the expected behavior is [`docs/playtime-window.md`](playtime-window.md).

## 1. Scope

**In scope (new feature):**
- Play start time ("Allow games from") per user
- Bed time ("Stop games after") per user
- Window enforcement in the sysmodule monitor, including windows crossing midnight
- Time tamper protection
- Overlay setup panel ("Playtime window" section) and main menu display
- Persistence in `settings.bin`

**Out of scope:**
- Blacklist, notifications content, history statistics (regression only)

## 2. Test environment

| Item | Requirement |
|---|---|
| Console | Nintendo Switch running Atmosphère (tested: 20.3.0 / 1.9.4) |
| Build | Sysmodule + overlay built from `feature-playtime-window` (v2.2.0) |
| Profiles | 2 user profiles: **User A** (limited) and **User B** (reference, no limits) |
| Game | Any game with a fast load (used for all "game running" preconditions) |
| Access | SD card readable (USB/MTP) to inspect `settings.bin` and `sysmodule.log` |

**Setup steps:**
1. Install the v2.2.0 sysmodule (`exefs.nsp` → `/atmosphere/contents/4200000000003103`) and the overlay (`parental_control.ovl` → `/switch/.overlays/`).
2. Reboot. Open the overlay → Admin (default PIN `A A A A`) → `Log level` = `DEBUG`.
3. For each test, reset User A's settings by setting: Daily = `0h 0mn` if "no daily limit" is not possible, otherwise use a high value (e.g. `12h 0mn`), and the window bounds as specified by the test.
4. The monitor loop runs every minute: **any behavior change takes effect within ~1 minute** after a precondition is met. Wait 2 minutes before concluding a failure.

**Verification means:**
- **Blocking**: the timeout screen is shown and the game is closed (working mode = `Blocking`).
- **Logs**: `/config/parental_control/sysmodule.log`, key lines:
  - `[Monitor] Current time HH:MM is outside the play window (HH:MM to HH:MM) for user NAME`
  - `[Monitor] Timeout for the user NAME`
  - `[Helpers] System time jump detected: ±N minutes, adjusting offset to N`
- **Settings**: `/config/parental_control/settings.bin` (keys `game_start_users`, `bedtime_users`, format `uid=minutes;...`).
- **Overlay**: main menu and setup panel contents.

Test result codes: **PASS / FAIL / BLOCKED / NEEDS-CONFIRMATION**.

## 3. Test cases

### A. Window start bound ("Allow games from")

| ID | Precondition (User A) | Steps | Expected |
|---|---|---|---|
| A1 | Start = 12:00, End = 0, no daily limit | Start a game at 11:50 | Game blocked (timeout screen). Log: "outside the play window" |
| A2 | Same | Start the same game at 12:05 | Game allowed, no block for 2 minutes. No "outside the play window" log line |
| A3 | Start = 0, End = 0 | Start a game at any time | Game allowed (no window restriction) |
| A4 | Start = 0, End = 0, no daily limit | Check `settings.bin` | `game_start_users` contains `uid=0` for User A (disabled) |
| A5 | Start = 12:00 | Start game at 11:50 (blocked), then wait until 12:01 | Block lifts: no new timeout screen within 2 minutes after 12:01 |

### B. Window end bound ("Stop games after")

| ID | Precondition (User A) | Steps | Expected |
|---|---|---|---|
| B1 | Start = 0, End = 22:00 | Start a game at 21:50 | Game blocked. Log: "outside the play window" |
| B2 | Same | Start the game at 21:55 | Game allowed for 2 minutes |
| B3 | Start = 0, End = 22:00, game running at 21:55 (allowed) | Change system time so the adjusted time passes 22:00 **without** a jump (normal flow) or re-test next day | Blocked after 22:00 |
| B4 | End = 0 | Start a game at 23:00 | Game allowed (end bound disabled) |

### C. Full window configurations

| ID | Precondition (User A) | Steps | Expected |
|---|---|---|---|
| C1 | Start = 08:00, End = 20:00 | Start a game at 07:00 / 09:00 / 19:00 / 21:00 | Blocked / allowed / allowed / blocked |
| C2 | Start = 20:00, End = 08:00 (crosses midnight) | Start a game at 21:00, 23:30, 00:30, 07:00, 12:00 | allowed / allowed / allowed / blocked / blocked |
| C3 | Start = 08:00, End = 08:00 (empty window) | Start a game at any time | Blocked all day |
| C4 | Start = 23:59, End = 00:01 (14-minute night window) | Start a game at 23:59 / 00:00 / 00:01 / 00:30 | allowed / allowed / blocked / blocked |
| C5 | Start = 0, End = 12:00 | Start a game at 00:30 / 11:59 / 12:00 | allowed / allowed / blocked |
| C6 | Start = 12:00, End = 0 | Start a game at 11:59 / 12:00 / 23:59 | blocked / allowed / allowed |

### D. Interaction with the daily limit

| ID | Precondition (User A) | Steps | Expected |
|---|---|---|---|
| D1 | Daily = 30 mn, window 00:00–24:00 (or unset), usage = 0 | Start a game, wait 31 minutes | Blocked (daily limit used up) |
| D2 | Daily = 30 mn, window Start = 22:00, End = 08:00 | Start a game at 14:00 | Blocked **because of the window**, not the daily limit. Log shows the window line, not just "Timeout" |
| D3 | Daily = 30 mn, window open at 14:00 | Start a game at 14:00 | Allowed until 30 minutes of usage are reached, then blocked |
| D4 | Daily = 30 mn, window closed (e.g. Start = 20:00), usage = 0 | Start a game at 14:00, confirm block, close the game | Usage history must **not** show 30 mn of usage for the blocked session. **NEEDS-CONFIRMATION**: `addToHistory` is called before the rule check — confirm with the maintainer whether play time consumed while blocked should count toward the daily limit |
| D5 | Daily = 10 mn, window open | Start a game, wait until blocked, then wait for the window to close | Block stays active (OR of the two conditions) |

### E. Clock tamper protection

| ID | Precondition (User A) | Steps | Expected |
|---|---|---|---|
| E1 | Window Start = 08:00, End = 20:00, game running at 09:00 (allowed) | Set the system clock to 22:00 (jump +3h) | Block is **not** lifted by the jump: game blocked within 1–2 minutes. Log: `System time jump detected: +180 minutes`. The offset compensates, so the adjusted time stays near 09:00 |
| E2 | Game running at 19:00 (allowed), window End = 20:00 | Set the system clock back to 10:00 (jump −9h) | No unblock: the game keeps running as allowed (adjusted time ≈ 19:00). Log: `System time jump detected: −540 minutes` |
| E3 | Game blocked at 07:00 (window Start = 08:00) | Set the system clock forward by 3 hours (attempt to "wait for" the window) | Still blocked: adjusted time ≈ 07:00 |
| E4 | Any test above | After the jump, read the log | Exactly one `System time jump detected` line per detected jump, with the correct sign and magnitude |
| E5 | Window Start = 08:00 | Reboot the console at 09:00 (offset state reset), start a game | Allowed: `lastTime`/offset state is reset on start, the real time is used again |

### F. Per-user isolation

| ID | Precondition | Steps | Expected |
|---|---|---|---|
| F1 | User A: window Start = 08:00, End = 20:00. User B: no settings at all | Start the same game at 22:00 with User A, then with User B | User A blocked, User B allowed |
| F2 | Same | Switch to User B at 22:00 with the game already running | Block released within 1–2 minutes (no timeout screen for User B) |
| F3 | User A: Daily = 10 mn, window unset. User B: Daily = 120 mn | Both play the same game the same day | Each user is limited independently (10 mn vs 120 mn) |
| F4 | After F1–F3 | Check `settings.bin` | `game_start_users` contains only User A's entry, never User B's (or `uid=0` for B if B was explicitly set) |

### G. Overlay UI

| ID | Steps | Expected |
|---|---|---|
| G1 | Admin → Set limits → select User A → open the panel | Two sections: "Play limits" (Daily) and "Playtime window" (Allow games from / Stop games after) |
| G2 | Navigate with ← / → across all six fields (daily h/mn, start h/mn, end h/mn) | Selection cycles Daily-h → Daily-mn → Start-h → Start-mn → End-h → End-mn → Daily-h, and the reverse with ← |
| G3 | Increase/decrease hours and minutes at the boundaries (23→24, −1 at 0, 59→60) | Hours wrap 0–23, minutes wrap 0–59 (valueRanged cycling) |
| G4 | Set Start = 13:37, End = 22:05, press B to exit, reopen the panel | Values are reloaded as 13:37 / 22:05 (round-trip through IPC + settings.bin) |
| G5 | With a game running and User A selected in the main menu, set Start = 10:00 (future time) | Main menu shows `Play from: 10:00` and `Bedtime: HH:MM` (only when the value is > 0) |
| G6 | Set Start = 0:00, End = 0:00 | Main menu shows neither `Play from:` nor `Bedtime:` line |
| G7 | Set a time to 0:00 for one bound only | The "Set time to 0h 0mn to disable." hint remains accurate; only the non-zero bound is shown in the main menu |
| G8 | Change the window while a game is running (e.g. set End to the current time) | New bound applies within 1–2 minutes (monitor reads settings every loop) |

### H. Persistence and migration

| ID | Steps | Expected |
|---|---|---|
| H1 | Set User A: Start = 09:00, End = 21:00, reboot the console | After reboot, the setup panel and `settings.bin` still hold 09:00 / 21:00 |
| H2 | Start a game 10 minutes before reboot (inside the window), reboot, restart the game | The adjusted-time state is reset (E5): behavior follows the real clock |
| H3 | Copy a `settings.bin` produced by v2.1 (no `game_start_users` key) to the console and reboot | Loads without error. Window disabled for all users. No crash, no data loss in the existing keys |
| H4 | After H3, set a window for User A and reboot | `game_start_users` created with the `uid=minutes` format, other keys intact |

### I. Regression (existing features)

| ID | Steps | Expected |
|---|---|---|
| R1 | No window set (both bounds 0:00), Daily = 30 mn | Behavior identical to v2.1: block after 30 mn of usage, notifications every 15 mn and every minute during the last 5 mn (with Ultrahand) |
| R2 | Enable/disable the parental control from the admin menu | Monitoring stops/resumes; no block when disabled |
| R3 | Usage history after a session | Session recorded with correct date, title and duration |
| R4 | Admin PIN: wrong PIN rejected, correct PIN accepted | No change |
| R5 | Main menu without a game running | Shows "No user / app started", warnings still displayed on top |
| R6 | Working mode / log level / show-remaining-time settings | No change, values persist |

## 4. Acceptance criteria

The feature is accepted when:

1. All tests of sections **A, B, C, E, F, G** are PASS.
2. All tests of sections **D, H, I** are PASS, except items marked **NEEDS-CONFIRMATION** which must have an explicit maintainer decision recorded (and the behavior documented in `docs/playtime-window.md` accordingly).
3. No regression in section I.
4. `sysmodule.log` at DEBUG level shows consistent window/tamper lines for every blocked session (no missing log, no crash, no duplicated entries per minute).

## 5. Known limitations to keep in mind

- The monitor granularity is **1 minute**: boundary tests (e.g. exactly 20:00) have a ±1 minute tolerance.
- The adjusted-time offset accumulates across the sysmodule lifetime and is reset when the sysmodule restarts (reboot).
- While a game is blocked, it is **counted** in the daily usage (see D4, NEEDS-CONFIRMATION).
- The overlay never blocks by itself: a block can only be observed with the sysmodule loaded and enabled.
