# Changelog

## Release 2.0

### Overlay

- Fixed small bugs

### Sysmodule

- Fixed bugs.
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