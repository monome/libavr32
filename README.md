# libavr32

[![Build Status](https://travis-ci.org/monome/libavr32.svg?branch=master)](https://travis-ci.org/monome/libavr32)

Common code shared between Monome Eurorack firmwares:

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
cd src
make
```

You should now have a `teletype.hex` file in the `src` directory.

To upload it, you'll need a [USB A-A][digikey] cable, then:

1. Power on your module with no USB cables attached, while holding down the front panel button
2. Connect the USB A-A cable to your computer and the module
3. Run `./flash.sh` in the `src` directory
4. The firmware should upload and verify itself. If it can't find the device, try repeating the steps.
5. Disconnect the USB A-A and power cycle the module to re-enable normal USB mode (otherwise your grid or keyboard will not be detected)

**You cannot overwrite the bootloader if you update via USB**

## `asf`

This folder contains the Atmel software framework, if you wish to make changes here, please make them using the [diet-asf][] repo.

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
