# NS Parental Control

NS Parental Control is a simple app that allows parents to set limits on their children’s use of the Nintendo Switch console.

*This is a proof-of-concept of an app and was made only for experimenting app development on the Nintendo Switch.* 

It can be used freely.

## Current version

1.0.0 (MVP)

## Current features 

Parental control has the following features:
- Set a daily limit for all users [PIN needed]. 
  - Each user has its own game time but it is defined the same for all users. 
  - The time limit is for all games.
- Set the parental access code (PIN).
- Show the remaining time (user).

## Coming features

The following features are in the backlog:
- Set a daily limit for users individually.
- Set a daily limit for each game, for each user.
- Show the remaining time for each game, for each user.

## Licence

The source code and the binaries are under [GPL v3 licence](LICENSE). 

You can:
- use it freely,
- modify it.

You must:
- share your changes by committing on this repository or your own fork.

You cannot :
- close the sources,
- sell the product,
- reuse the source code in a commercial product,
- use your own modified version.

### Dependencies

Libraries linked or code reused:
- AES and SHA256 from **Brad Conte** ([GitHub](https://github.com/B-Con/crypto-algorithms)) - no licence.
- Tesla,
- libNX.

## Build

This section explains how to create and install the binary for NS Parental Control.

### Architecture

This product relies on 3 components:
1 - a sysmodule that monitors the games usage and notifies when limit is reached.
2 - an overlay that shows on demand information about the limits and permits setup of the limits.

The sysmodule and the overlay share a common database file. 

### Pre-requisistes

- Atmosphere installed
- Development computer with `devkitPro` and `devkitA64` (see below)
- `libnx` (Switch homebrew SDK) installed via devkitPto
- Tesla menu installed on the Switch

### devkitPro and devkitA64 installation

#### Linux Debian

```
sudo apt install git make cmake build-essential
wget https://apt.devkitpro.org/devkitpro-keyring_20230702_all.deb
sudo dpkg -i devkitpro-keyring_20230702_all.deb
sudo apt update
sudo apt install devkitpro-dev
sudo apt install devkitarm-dev devkita64-dev
``` 

#### Mac OS
 
- Download and install the latest pacman release from [GitHub](https://github.com/devkitPro/pacman/releases).
- Follow the instructions on [this page](https://devkitpro.org/wiki/devkitPro_pacman#macOS) to install the needed packages.

### Compilation

- Go to the root directory of the project (usually `NSParentalControl`)
- Choose the component you want to build by entering its directory (for example `$ cd sysmodule` for the sysmodule)
- Run `make`

### Installation

Install the sysmodule:
```
/atmosphere/contents/0004000000000000/exefs.nsp
/atmosphere/contents/0004000000000000/toolbox.json
/atmosphere/contents/0004000000000000/flags/boot2.flag
```

Install the overlay:
*TODO*

### Auto-start Parental Control

- On the SD card, create the folder `/atmosphere/contents/0004000000000000/ParentalControl/` (if 0004000000000000 already exists, choose another one randomly).
- Copy the file `ParentalControl.nro` into the new folder and rename it to `main.nro`.
- Create a new folder `/atmosphere/contents/0004000000000000/ParentalControl/meta/`.
- Copy the file `meta.ini` provided into this folder.

## References

https://github.com/switchbrew/switch-examples/tree/master