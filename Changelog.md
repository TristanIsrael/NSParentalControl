# Changelog

## Release 2.4.0

### Overlay

- New `Blocked games` entry in the Admin menu: choose a user, then block or allow each game installed on the console
- In the limits setup screen, holding up or down changes the value continuously, faster and faster (about 7, then 15, then 30 steps per second)

### Sysmodule

- A game blocked for the user is stopped: the user is notified that the game is not allowed, then the game is closed after the usual countdown (detected within 5 seconds of its launch, then confirmed 10 seconds later with the current user)
- The blocked games commands of the IPC contract (`GetBlacklistedTitlesCount`, `GetBlacklistedTitle`, `AddTitleToBlacklist`, `RemoveTitleFromBlacklist`) are implemented
- The blocked games setting is parsed without throwing: an invalid title ID is ignored instead of aborting the sysmodule

## Release 2.3.3

### Sysmodule

- Fixed the console sleep being detected as a system clock change: each wake-up shifted the tamper-protected time (by more than 15 hours after a few days), so the playtime window was evaluated on a wrong time. The real elapsed time is now measured with the steady clock, which keeps running while the console sleeps and cannot be changed by the user
- Fixed the change of day at midnight being detected as a clock change (-1439 minutes)
- Fixed a game being stopped because of the previous user: many games are launched before the user is selected. The block is now confirmed 10 seconds later with the current user, and the countdown is cancelled when the current user is allowed to play
- The tamper-protected time state is protected by a mutex (it is also read by the `GetAdjustedCurrentTime` IPC command)

## Release 2.3.2

### Sysmodule

- Fixed the playtime restarting from zero after a reboot: the history is now loaded before the usage is recorded (previously the usage already recorded that day was ignored and the history file was overwritten with the new entry only)
- Fixed concurrent accesses to the history between the monitor and the IPC service (the history is now loaded, read and updated under its mutex)
- An unreadable history file is moved to `sessions.corrupted.bin` and a new history is started, instead of blocking the usage recording
- The game is closed from the HOME menu with a virtual controller (HOME, X, A), so that the system no longer reports "The software was closed because an error occurred"; `ns:dev` then `pm:shell` are used as a fallback
- A game launched again while the user is not allowed to play is detected within 5 seconds instead of up to one minute
- The monitoring loop is scheduled on fixed deadlines (one counted minute is one real minute) and no longer spins without pausing when the usage cannot be recorded
- The usage time request now always replies a value, even when there is no current user
- Fixed an SD card file system handle leak when Ultrahand notifications are disabled
- Fixed the notifications always being in English: the console language was read after the connection to the service manager had been closed, so the SET service could not be opened

### Tools

- Added `scripts/migrate_json_to_bin.py` to convert the 1.x data files (`settings.json`, `sessions.json`) into the binary files, so that upgrading from 1.x keeps the PIN, the limits and the history

## Release 2.3.1

### Sysmodule

- Fixed a bug where, once the service was disabled, the monitoring thread exited and never resumed: after re-enabling the parental control, the playtime was no longer counted and the game was no longer interrupted (a reboot was previously required)
- Fixed a potential crash when the enable state was changed over IPC before the monitor thread had started

## Release 2.3

### Overlay

- The interface is displayed in the console language: English, French and Simplified Chinese are supported (other console languages fall back to English)
- The playtime window times (game start and bed time) are always displayed with two digits in hours and minutes (HH:MM, e.g. 01:09)

### Sysmodule

- The notifications are displayed in the console language: English, French and Simplified Chinese are supported (other console languages fall back to English)
- The language is detected from the console settings at startup; change the console language and restart the sysmodule to apply it

## Release 2.2

### Overlay

- Setup limits panel: new "Playtime window" section grouping the play start time ("Allow games from") and the bed time ("Stop games after")
- Main menu: shows the play start time ("Play from: HH:MM") when set

### Sysmodule

- New per-user setting: the earliest allowed play time (games are stopped before this time)
- The game is now stopped when the daily limit is used up, or when the current time is outside the playtime window (before the play start time or after the bed time), including windows crossing midnight
- The time used for the checks is protected against manipulations of the system clock

## Release 2.1

### Overlay

- Setup limits panel: bed time configuration (hours / minutes)
- Main menu: shows the bed time

### Sysmodule

- New per-user bed time: the game is stopped after the bed time

## Release 2.0

### Overlay

- Fixed small bugs

### Sysmodule

- Fix the bug that prevented from setting the users limits ("176h").
- Fix crashes.
- Changed the database from JSON to binary.
- Replaced the limit screen by notifications and the forced shutdown of the the game.
- Reduced the memory reserver from 2500 MB to 512 MB for the heap.

## Release 1.1

### Overlay

- #24 - Show the sysmodule current state (started / enabled / ...)
- #19 - Show the remaining time at the user level in the overlay, including when not playing a game
- #14 - Show usage history for all users

### Sysmodule

- #21 - After timeout blocking reboot to the payload
- #18 - When using takeover for a homebrew the title is not detected correctly
- #15 - No information (nor time couting) are shown in the overlay for some games