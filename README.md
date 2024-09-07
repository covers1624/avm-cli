# AVM-CLI

A CLI interface for interacting with and controlling AVerMedia devices on Linux.

## Features

- Enabling/Disabling HDCP.
- Setting LED brightness.

## Supported capture cards

If your AVerMedia uvc capture card is not listed here, or has features which need to be supported,
please open an Issue or PR.

### Known working
- AVerMedia Live Gamer HD 2

## Usage

```
$ ./avm-cli --help
Usage: avm-cli [OPTION...] 
CLI application for controlling AVerMedia UVC devices. Enable/Disable HDCP,
change the LED brightness.

  -s, --status               Print the current device status.
  -d, --device=DEVICE        The video device to control.
  -h, --hdcp=HDCP            Set the current hdcp status. [1/0, on/off, yes/no]
                            
  -l, --led=LED              Set the current led status. [0, 25, 50, 75, 100]
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

```

Enabling/Disabling HDCP: `./avm-cli --device /dev/video0 --hdcp off`

Setting LED Brightness: `./avm-cli --device /dev/video0 --led 0`


## Building

Requirements:
- meson
- GCC/Clang

```
$ meson setup build --buildtype release
$ cd build
$ ninja
```
