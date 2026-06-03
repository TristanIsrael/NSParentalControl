## Debug

https://gist.github.com/jam1garner/c9ba6c0cff150f1a2480d0c18ff05e33
https://github.com/misson20000/twili

### Analyze a crash report

Collect the `PC` and return addresses:
- PC = crash location (pctrl + <address>)

Run `$ /opt/devkitpro/devkitA64/bin/aarch64-none-elf-addr2line -a -p -f -e pctrl.elf <address>`
