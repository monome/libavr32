# libavr32

[![Build Status](https://travis-ci.org/monome/libavr32.svg?branch=master)](https://travis-ci.org/monome/libavr32)

Common code shared between Monome Eurorack firmwares:

* [ansible][]
* [earthsea][]
* [kria][]
* [meadowphysics][]
* [teletype][]
* [whitewhale][]

## Getting starting with development

If you want to build your own copy of any of the firmwares you'll need the Atmel AVR-32 toolchain installed.

### OSX

If you're on OSX you can use the [avr32-toolchain][] repo to build your own. You'll need [Homebrew][homebrew] installed.

To install on OSX in `~/avr32-tools`:

```bash
brew install mpfr gmp libmpc texinfo dfu-programmer
git clone https://github.com/monome/avr32-toolchain.git
cd avr32-toolchain
PREFIX=$HOME/avr32-tools make install-cross
# go make a cup of tea, this will take a while...
```

### Linux

If you're on Linux, download the toolchain and the headers from [Atmel][atmellinux]. You need to dowload:
 - Atmel AVR 32-bit Toolchain 3.4.3 - Linux 64-bit (`avr32-gnu-toolchain-3.4.3.820-linux.any.x86_64.tar.gz`)
 - Atmel 32-bit Toolchain (3.4.3) 6.2.0.742 - Header Files (`avr32-headers-6.2.0.742.zip`)

The headers need to be installed in the correct location.

To install on Linux in `~/avr32-tools`:

```bash
tar xvfz avr32-gnu-toolchain-3.4.3.820-linux.any.x86_64.tar.gz
mv avr32-gnu-toolchain-linux_x86_64 $HOME/avr32-tools
unzip avr32-headers-6.2.0.742.zip -d $HOME/avr32-tools/avr32/include
```

You should also install `dfu-programmer` from your package manager.

### Building a firmware

Let's build and upload a copy of the [teletype][] firmware

First clone the repo, we need to clone recursively to bring in the `libavr32` submodule.
```bash
git clone --recursive https://github.com/monome/teletype.git
cd teletype

```

Let's build it, we need to make sure that `avr32-gcc` and company are available on our path. Assuming they are installed in `~/avr32-tools`:
```bash
export PATH="$HOME/avr32-tools/bin:$PATH"
cd module
make
```

You should now have a `teletype.hex` file in the `src` directory.

To upload it, you'll need a [USB A-A][digikey] cable, then:

1. Power on your module with no USB cables attached, while holding down the front panel button
2. Connect the USB A-A cable to your computer and the module
3. Run `./flash.sh` in the `module` directory
4. The firmware should upload and verify itself. If it can't find the device, try repeating the steps.
5. Disconnect the USB A-A and power cycle the module to re-enable normal USB mode (otherwise your grid or keyboard will not be detected)

**You cannot overwrite the bootloader if you update via USB**

## `asf`

This folder contains the Atmel software framework, if you wish to make changes here, please make them using the [diet-asf][] repo.

## AVR32 development

The modules use AVR32 MCUs from Atmel, either the [AT32UC3B0512][] or the [AT32UC3B0256][].

| module                                          | MCU              | RAM  | ROM   | default stack size | default NVRAM size |
|-------------------------------------------------|------------------|------|-------|--------------------|--------------------|
| [ansible][], [teletype][]                       | [AT32UC3B0512][] | 96kb | 512kb | 8kb                | 256kb              |
| [earthsea][], [meadowphysics][], [whitewhale][] | [AT32UC3B0256][] | 32kb | 256kb | 4kb                | 128kb              |

### Useful AVR32 documents

- [AVR32006: Getting started with GCC for AVR32](http://www.atmel.com/Images/doc32074.pdf): useful information on GCC flags and speed and size optimisation.
- [AVR32795: Using the GNU Linker Scripts on AVR UC3 Devices](http://www.atmel.com/images/doc32158.pdf): detailed instructions on controlling memory layout.
- [AT08569: Optimizing ASF Code Size to Minimize Flash and RAM Usage](http://www.atmel.com/Images/Atmel-42370-Optimizing-ASF-Code-Size-to-Minimize-Flash-and-RAM-Usage_ApplicationNote_AT08569.pdf).

### (very) In depth AVR32 documentation

There is a lot of overlap between these documents, the table of contents is at the back.

- [AT32UC3B Series Complete](http://www.atmel.com/Images/doc32059.pdf)
- [AVR32 Architecture Manual](http://www.atmel.com/Images/doc32000.pdf)
- [AVR32UC Technical Reference Manual](http://www.atmel.com/Images/doc32000.pdf)

### RAM and ROM usage

In short run `avr32-size -A <module name>.elf`. All free RAM is allocated to the `.heap`. ROM size is approximately `.data` + `.rodata` + `.text`. NVRAM is given by `.flash_nvram`. See [here][avr32-ram-and-rom-usage] for a more in depth discussion.


[ansible]: https://github.com/monome/ansible
[earthsea]: https://github.com/monome/earthsea
[kria]: https://github.com/monome/kria
[meadowphysics]: https://github.com/monome/meadowphysics
[teletype]: https://github.com/monome/teletype
[whitewhale]: https://github.com/monome/whitewhale
[diet-asf]: https://github.com/monome/diet-asf
[avr32-toolchain]: https://github.com/monome/avr32-toolchain
[atmellinux]: http://www.atmel.com/tools/atmelavrtoolchainforlinux.aspx
[homebrew]: http://brew.sh/
[digikey]: http://www.digikey.com/product-detail/en/101-1020-BE-00100/1175-1035-ND/3064766
[AT32UC3B0256]: http://www.atmel.com/devices/AT32UC3B0256.aspx
[AT32UC3B0512]: http://www.atmel.com/devices/AT32UC3B0512.aspx
[avr32-ram-and-rom-usage]: http://samdoshi.com/post/2016/10/avr32-ram-and-rom-usage/
