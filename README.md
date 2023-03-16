[![CI-Release](https://github.com/wiiu-env/gdbstub_plugin/actions/workflows/ci.yml/badge.svg)](https://github.com/wiiu-env/gdbstub_plugin/actions/workflows/ci.yml)

# GDBStub for the Wii U

This is a plugin which provides a gdbstub for debugging Wii U software, including homebrew. 

It's based on the gdbstub that's inside the `coreinit.rpl` and present on any Wii U. This plugin patches several functions to allow usage of the stub on retail consoles.

With these patches some bugs have been fixed (e.g. basic `vCont` support and escape packet contents) and support for multiple queries have been added. 
Memory access is now done completely via the KernelModule which bypasses the MMU, which allow us to write/read anywhere.

For research purposes it's planned to reimplement (the important parts of) the `coreinit.rpl` gdbstub step by step.

## Features
- Hardware breakpoint support (maximum one hw breakpoint is supported at once).
- Support up to 512 software breakpoints at the same time.
- Hardware watchpoints for read/write access of data with 8 byte accuracy (maximum one hw watchpoint is supported at once).
- Stepping to the next instruction (only the for currently active threads) 
- Memory/register reading/writing.
- Implemented the optional `qXfer:features:read` and `qXfer:threads:read` query.
- Interrupting the execution at any time via `CTRL+C`
- See the `main.h` for further configuration options via macros.

## Requirements / Limitations:
- Using [MochaPayload](https://github.com/wiiu-env/MochaPayload) v0.2 or higher or something else that supports the DK_PChar API. If not using mocha, you probably need to adjust the `makeOpenPath` replacement.
- Any homebrew you want to debug needs to be built with [wut](https://github.com/devkitPro/wut) 1.2.0-2 or higher. Official software works too, but you probably won't have any symbols.
- When the gdbstub is loaded, it'll directly start to wait for a connection via TCP. It's recommended to load the plugin when needed via the network instead of placing it permanently on the sd card. For loading plugins at runtime you can use the [wiiload plugin](https://github.com/wiiu-env/wiiload_plugin). 
- The console will **not** respond to any other input while waiting for a connection. Please connect to the stub or force-shutdown the console (hold the power button of the **console** for at least 4 seconds)
- The gdbstub will stay loaded and active until the console is shut down. It's possible to change or reload the application.

## Recommended:
- Using the [USBSerialLogger](https://github.com/wiiu-env/USBSerialLogger) module to have output via serial while debugging. For this you need a USB serial cable with certain chipset. The USBSerialLogger repo for more information.
- Build the homebrew apps with -O0, make sure to add the `-g` compiler flag for creating debug builds with DWARF information.

## Usage
- Boot your console into a [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) compatible environment (e.g. [Aroma](https://github.com/wiiu-env/Aroma))
- Start the homebrew app you want to debug.
- Load the gdbstub_plugin via wiiload.
- Now the homebrew app will restart and wait for a connection. (When using the usb serial logging it should print `[+-*WAITING FOR DEBUGGER*-+]`)
- Connect to the gdbstub via TCP at port 3000.
- When you don't load any symbol file, you might have to force the endianness via `set endian big`

Basic settings for gdb:
```
set arch powerpc:750
set remotetimeout 30
set tcp connect-timeout 60
set print thread-events on
set disassemble-next-line on
set remote hardware-watchpoint-limit 1
set remote hardware-breakpoint-limit 1
target remote tcp:192.168.178.123:3000
```

## Building
In order to be able to compile this, you need to have devkitPPC installed
[devkitPPC](https://devkitpro.org/wiki/Getting_Started) with the following
pacman packages installed.

```
(sudo) (dkp-)pacman -Syu --needed wiiu-dev
```

Make sure the following environment variables are set:

```
DEVKITPRO=/opt/devkitpro
DEVKITPPC=/opt/devkitpro/devkitPPC
```

Also make sure to
install [wut](https://github.com/wiiu-env/wut), [WiiUPluginSystem](https://github.com/wiiu-env/WiiUPluginSystem), [libkernel](https://github.com/wiiu-env/libkernel)
and [libwupsbackend](https://github.com/wiiu-env/libwupsbackend).

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t gbdstubplugin-builder

# make
docker run -it --rm -v ${PWD}:/project gbdstubplugin-builder make

# make clean
docker run -it --rm -v ${PWD}:/project gbdstubplugin-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src ghcr.io/wiiu-env/clang-format:13.0.0-2 -r ./src -i`