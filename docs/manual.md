# User manual

This document describes how to use Parental Control for the Switch.

## What happens when the limit has been reached?

The last version (> 2.0.0) has a different behaviour from the versions before, it shows a notification every 10 seconds during the last minute and finally it closes the running game. The versions before showed a bug "Time's up" screen and forced the user to reboot the console.

## Overlay

Interactions with the Parental Control are made thru the overlay.

First open the overlay using the combo defined in your Overlay manager.

### Main menu

When there are warnings they are shown at the top of the list:
```
ⓘ Database tampered! ---> Means that the database or settings data have been manually modified
ⓘ Database need upgrade! ---> Means that the database format has changed and you have to change a value in the admin section to upgrade it (for example reset your PIN to the same value).
```

Then when no game has been started, the menu shows:

```
No user / app started
Usage history
Admin
```

When a game has been started, the menu shows:

```
User: [username]
Playing: [game title]
Usage: 0 mn
Remaining: 1h 30 mn
Usage history
Admin
```

### Usage history

The usage history is divided into different sections showing statistics on the usage. *Currently only the current day usage is shown*.

When entering the menu, a submenu appears showing the list of all the users.

After selecting a user, the statistics appear:

```
Today
  Usage         1h 30mn
  Remaining     0h 0mn
```

### Admin menu

The admin menu is protected with a PIN.. The default PIN is `A A A A`.

```
Enabled                 Enabled
Working mode            Blocking
Notify remaining time   Activate
Log level               INFO
Set PIN
Set limits

Versions
  Overlay               v1.2
  Sysmodule             v1.2
```

- `Enabled` is a switch for enabling and disabling the parental control. When it disabled it does not count play time anymore.
- `Working mode` lets you define whether the system will be blocked at the end of play time or the play time is only shown on screen as a badge (Currently the only value is `Blocking`).
- `Notify remaining time` is a switch for enabling notifications about the remaining time. When enabled, a notification is shown every 15 minutes, and every minute during the last 5 minutes. **This feature is only available when Ultrahand overlay is used**.
- `Log level` lets you define the log level to DEBUG or INFO in the sysmodule and the overlay. Use DEBUG only when needed because it can generates big files.
- `Set PIN` opens a new menu for setting the Admin PIN (see [Admin PIN setup](#admin-pin-setup)).
- `Set limits` lets you define the play time limits (See [Setup limits](#setup-limits)).

#### Admin PIN setup

This screen lets you define a PIN for the Administrator of the Parental Control. The PIN can be defined with any pad button or combination.

#### Setup limits

This screen lets you define the unique play time limit for all the players. Each player has its own play time but the limit is the same for every player.

In case you want more time to play for a single player you will have to disable the parental control temporarilty.