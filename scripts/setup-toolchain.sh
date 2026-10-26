#!/usr/bin/env bash
#
# setup-toolchain.sh
#
# Install the devkitPro Switch (devkitA64) toolchain + libnx on a Debian-based
# system, following the official devkitPro pacman procedure:
#   https://devkitpro.org/wiki/devkitPro_pacman  (section "Debian and derivatives")
#
# IMPORTANT: this must be run from a network that can reach devkitpro.org.
# From datacenter / cloud egress IPs, devkitpro.org is blocked by Cloudflare
# (HTTP 403 "you have been blocked") regardless of User-Agent. The preflight
# below detects that and aborts with a clear message.
#
set -euo pipefail

UA="dkp-apt"          # User-Agent used by the official installer (see wiki)
DEVKITPRO="/opt/devkitpro"
KEYRING_URL="https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.zst"
INSTALLER_URL="https://apt.devkitpro.org/install-devkitpro-pacman"

echo "==> [0/4] Preflight: is devkitpro.org reachable from this network?"
code="$(curl -sS -m 20 -A "$UA" -o /dev/null -w '%{http_code}' "$KEYRING_URL" 2>/dev/null || echo 000)"
if [ "$code" != "200" ]; then
    echo "ERROR: $KEYRING_URL returned HTTP $code."
    echo "       devkitpro.org is very likely blocking this IP (Cloudflare WAF)."
    echo "       User-Agent is NOT the issue - run this from a network that can"
    echo "       reach devkitpro.org (a normal/residential connection usually works)."
    exit 1
fi
echo "    OK (HTTP $code)"

echo "==> [1/4] Downloading + running the official dkp-pacman Debian installer"
workdir="$(mktemp -d)"
trap 'rm -rf "$workdir"' EXIT
cd "$workdir"
wget -U "$UA" -q "$INSTALLER_URL" -O install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
echo "    Installer saved to $workdir/install-devkitpro-pacman"
echo "    (The wiki advises reading it before running - please do.)"
sudo ./install-devkitpro-pacman

echo "==> [2/4] Refreshing the devkitPro package database"
sudo dkp-pacman -Sy

echo "==> [3/4] Installing the 'switch-dev' group (devkitA64 toolchain + libnx)"
sudo dkp-pacman -S --noconfirm switch-dev

echo "==> [4/4] Done. Environment to export (add to ~/.bashrc):"
cat <<EOF

  export DEVKITPRO=$DEVKITPRO
  export DEVKITARM=\$DEVKITPRO/devkitARM
  export DEVKITPPC=\$DEVKITPRO/devkitPPC
  export PATH="\$PATH:\$DEVKITPRO/bin"

Then build the project:
  cd NSParentalControl/sysmodule && make    # -> out.nosync/exefs.nsp
  cd NSParentalControl/overlay   && make    # -> parental_control.ovl
EOF
