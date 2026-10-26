# Changelog

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