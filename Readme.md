# NS Parental Control

[![platform](https://img.shields.io/badge/platform-Switch-898c8c?logo=C++.svg)](https://gbatemp.net/forums/nintendo-switch.283/?prefix_id=44)
[![language](https://img.shields.io/badge/language-C++-ba1632?logo=C++.svg)](https://github.com/topics/cpp)
[![GPLv3 License](https://img.shields.io/badge/license-GPLv3-189c11.svg)](https://www.gnu.org/licenses/old-licenses/gpl-3.0.en.html)
[![Latest Version](https://img.shields.io/github/v/release/TristanIsrael/NSParentalControl?label=latest&color=blue)](https://github.com/TristanIsrael/NSParentalControl/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/TristanIsrael/NSParentalControl/total?color=6f42c1)](https://github.com/TristanIsrael/NSParentalControl/graphs/traffic)
[![GitHub issues](https://img.shields.io/github/issues/TristanIsrael/NSParentalControl?color=222222)](https://github.com/TristanIsrael/NSParentalControl/issues)
[![GitHub stars](https://img.shields.io/github/stars/TristanIsrael/NSParentalControl)](https://github.com/TristanIsrael/NSParentalControl/stargazers)

NS Parental Control is a simple parental control system for the Nintendo Switch which does not require Internet access or a smartphone to operate.

It can be used freely.

The user manual can be found [here](docs/manual.md).

## Dependencies

**Needs a CFW (Atmosphère).**

**Tested with**

| Firmware | Atmosphere | State |
|--|--|--|
| 20.3.0 | 1.9.4 | Working |
| 21.2.0 | 1.10.2 | Test in progress |
| 21.2.0 | 1.12.1 | Games randomly stop (Atmosphere?) |

Linked with:
- libnx 4.10.0 
- libultrahand 2.2.0

## Table of Contents

- [Users categories](#users-categories)
- [Current features](#current-features)
- [Coming features](#coming-features)
- [Screenshots](#screenshots)
- [Licence](#licence)
- [Installation](#installation)
- [Build and install](#build-and-install)
- [References](#references)

## Users categories

The following users are involved:
- Gamers: the persons who want to play games.
- Administrator: the person who defines the rules and sets the limits.

## Current features 

Parental control has the following features:

**Gamers**
- Check the played time (since 1.0)
- Check the remaining play time (since 1.0)
- When the time is out, the system is blocked (since 1.0)
- Be notified every 15 minutes and every minute in the last 5 minutes (since 1.2)
- Get usage history (since 1.2)

**Administrator** (protected by a PIN code)
- Define a PIN to protect setup access - *default PIN is A A A A* (since 1.0)
- Enable or disable the parental control (since 1.0)
- Enable or disable the notifications (since 1.2)
- Choose the log level between Debug and Information (since 1.2)
- Define individual limits (since 1.3)
- Define bed time (since 2.1)
- Define the playtime window: earliest allowed play time and bed time (since 2.2)

## Coming features

Coming features are in the [GitHub project](https://github.com/users/TristanIsrael/projects/6/views/1) page.

## Screenshots

![Tesla Main menu](docs/2025102321560900.jpg)
**Tesla Main Menu**

![Parental control main menu](docs/2025102321561600.jpg)
**Parental control main menu**

![Parental control admin PIN](docs/2025102321562400.jpg)
**Parental control admin PIN**

![Parental control admin menu](docs/2025102321564400.jpg)
**Parental control admin menu**

![Parental control limits setting](docs/2025102321565700.jpg)
**Parental control limits settings**

## Licence

The source code and the binaries are under [GPL v3 licence](LICENSE). 

You can:
- use it freely,
- modify it.

You must:
- share your changes by committing on this repository or your own fork.
- credit the original project in your forks.

You are not allowed to:
- close the sources,
- sell the product,
- reuse the source code in a commercial product.

### Dependencies

Libraries linked or code reused:
- AES and SHA256 from **Brad Conte** ([GitHub](https://github.com/B-Con/crypto-algorithms)) - no licence.
- Tesla,
- libNX.

## Installation

1. Install the required [**Tesla Menu**](https://switch.hacks.guide/homebrew/tesla-menu.html) or [**Ultrahand Overlay**](https://github.com/ppkantorski/Ultrahand-Overlay).

⚠️ Since version 2.0 you have to install **[Ultrahand Overlay](https://github.com/ppkantorski/Ultrahand-Overlay)**. Please look at its documentation to know how to install it and how to enable the notifications.

2. Download the latest release from [GitHub](https://github.com/TristanIsrael/NSParentalControl/releases/latest).

**The sysmodule is provided as a ZIP file which can be directly unzipped in the folder `/atmosphere/contents/`**

Here are the files and their destination:

| File | Destination | Description |
|--|--|--|
| exefs.nsp | /atmosphere/contents/4200000000003103 | The core of the system as a *sysmodule* |
| toolbox.json | /atmosphere/contents/4200000000003103 | A description file for Atmosphère |
| boot2.flag | /atmosphere/contents/4200000000003103/flags | Mandatory file to make the sysmodule start at boot |
| parental_control.ovl | /switch/.overlays | The overlay |

After copying the files, reboot the console.

## Build and install

This section explains how to create and install the binary for NS Parental Control.

### Architecture

This product relies on 2 components:
1 - a sysmodule that monitors the games usage and notifies when limit is reached. 
2 - an overlay that shows on demand information about the limits and permits setup of the limits.

The sysmodule and the overlay share a common database file. 

### Pre-requisistes for runtime

- Atmosphere installed
- Tesla menu installed or Ultrahand overlay for notification

### Pre-requisites for build

- A development computer (Linux) with the devkitPro **devkitA64** toolchain and **libnx** (the Switch homebrew SDK) installed — see below.
- The Switch targets cannot be built without the devkitPro toolchain. The host unit tests (`make -C tests`) are the exception: they only need a standard C++20 compiler (e.g. `g++`) and no Switch SDK.

### Installing the build environment (devkitPro + libnx)

The toolchain and libnx are provided by devkitPro. The recommended installation method is **devkitPro pacman** (`dkp-pacman`), documented in the [official devkitPro wiki](https://devkitpro.org/wiki/devkitPro_pacman).

On Debian/Ubuntu based systems, the whole setup can be run with the provided script:

    bash scripts/setup-toolchain.sh

The script follows the official *Debian and derivatives* procedure: it installs `dkp-pacman`, refreshes the devkitPro package database, and installs the `switch-dev` group (devkitA64 toolchain + libnx). It first checks that `devkitpro.org` is reachable and aborts with a clear message if not.

> **Network note:** `devkitpro.org` is protected by Cloudflare and **blocks datacenter / cloud egress IPs** (HTTP 403, regardless of User-Agent). Run the script from a normal (residential / office) connection. When it happens, this is a network limitation, not a configuration problem.

Alternatively, follow the manual steps of the [devkitPro pacman wiki](https://devkitpro.org/wiki/devkitPro_pacman).

After installation, export the environment variables (the script prints them):

    export DEVKITPRO=/opt/devkitpro
    export DEVKITARM=/opt/devkitpro/devkitARM
    export DEVKITPPC=/opt/devkitpro/devkitPPC
    export PATH="$PATH:/opt/devkitpro/bin"

### Build the sysmodule

Once the code is downloaded or cloned do the following:

Go to the directory `NSParentalControl/sysmodule`

Run the command `$ make`. At the end of the build, the `out.nosync` directory contains the file `exefs.nsp`.

This file should be copied in the directory `/atmosphere/contents/4200000000003103` of the SD card of the Switch.

### Build the overlay

Go to the directory `NSParentalControl/overlay`.

Run the command `$ make`. At the end of the compilation process, the directory contains a file name `parental_control.ovl`. 

Copy this file in the directory `/switch/.overlays/` in the SD card of the Switch.

### Auto-start Parental Control

In order for the parental control to load on startup you have to create a new folder named `flags` into `/atmosphere/contents/4200000000003103`. 

In this folder create an empty file named `boot2.flag`.

### Versioning

The version follows the `MAJOR.MINOR.PATCH` scheme (e.g. `2.2.0`):

- A bug fix bumps the last digit (patch) only.
- A new feature bumps the second digit (minor) and resets the last digit to 0.
- The first digit (major) is bumped only by the product maintainer.

## References

https://github.com/switchbrew/switch-examples/tree/master

## Credits

- Niels Lohmann <https://nlohmann.me> for JSON C++ library.
- @SciresM for Atmosphère.
- Sean Barrett for STB Truetype library.
- @WerWolv for [Tesla library](https://github.com/WerWolv/libtesla).
- @ppkantorski for [Ultrahand Overlay](https://github.com/ppkantorski/Ultrahand-Overlay).
- @tallbl0nde for IPC Server (from TriPlayer)
