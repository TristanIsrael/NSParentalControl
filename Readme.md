# NS Parental Control

NS Parental Control is a simple app that allows parents to set limits on their children’s use of the Nintendo Switch console.

*This is a proof-of-concept of an app and was made only for experimenting app development on the Nintendo Switch.* 

It can be used freely.

## Licence

The source code and the binaries are under [GPL v3 licence](LICENSE). 

You can:
- use it freely,
- modify it.

You must:
- share your changes by committing on this repository or your on fork.

You cannot :
- close the sources,
- sell the product,
- reuse the source code in a commercial product,
- use your own modified version.

### Dependencies

Libraries linked or code reused:
- AES and SHA256 from **Brad Conte** ([GitHub](https://github.com/B-Con/crypto-algorithms)) - no licence.
- IMGUI

## Build

This section explains how to create and install the binary for NS Parental Control.

### Pre-requisistes

- Atmosphere installed
- Development computer with `devkitPro` and `devkitA64` (see below)
- `libnx` (Switch homebrew SDK) installed via devkitPto
- Homebrew menu installed on the Switch

### devkitPro and devkitA64 installation

```
sudo apt install git make cmake build-essential
wget https://apt.devkitpro.org/devkitpro-keyring_20230702_all.deb
sudo dpkg -i devkitpro-keyring_20230702_all.deb
sudo apt update
sudo apt install devkitpro-dev
sudo apt install devkitarm-dev devkita64-dev
``` 

### Compilation

- Go to the root directory of the project (usually `SwitchParentalControl`)
- Run `make`

### Installation

- Copy `ParentalControl.nro` on the SD car in a new directory called `/switch/ParentalControl/`.
- Launch `Homebrew Menu` on the switch
- Start `ParentalContro.nro`

### Auto-start Parental Control

- On the SD card, create the folder `/atmosphere/contents/0004000000000000/ParentalControl/` (if 0004000000000000 already exists, choose another one randomly).
- Copy the file `ParentalControl.nro` into the new folder and rename it to `main.nro`.
- Create a new folder `/atmosphere/contents/0004000000000000/ParentalControl/meta/`.
- Copy the file `meta.ini` provided into this folder.