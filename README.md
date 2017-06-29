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

```
bash
brew install mpfr gmp libmpc texinfo dfu-programmer
git clone https://github.com/monome/avr32-toolchain.git
cd avr32-toolchain
PREFIX=$HOME/avr32-tools make install-cross
# go make a cup of tea, this will take a while...
```

If you encounter an error make sure you have the Xcode command line tools installed. They can be installed by typing `xcode-select --install` (if they are already installed it will print an error message).

### Linux

If you're on Linux, download the toolchain and the headers from [Atmel][atmellinux]. You need to dowload:
 - Atmel AVR 32-bit Toolchain 3.4.3 - Linux 64-bit (`avr32-gnu-toolchain-3.4.3.820-linux.any.x86_64.tar.gz`)
 - Atmel 32-bit Toolchain (3.4.3) 6.2.0.742 - Header Files (`avr32-headers-6.2.0.742.zip`)

The headers need to be installed in the correct location.

To install on Linux in `~/avr32-tools`:

```
bash
tar xvfz avr32-gnu-toolchain-3.4.3.820-linux.any.x86_64.tar.gz
mv avr32-gnu-toolchain-linux_x86_64 $HOME/avr32-tools
unzip avr32-headers-6.2.0.742.zip -d $HOME/avr32-tools/avr32/include
```

You should also install `dfu-programmer` from your package manager.

### Windows

Install bash: https://msdn.microsoft.com/en-us/commandline/wsl/install_guide

Install dependencies to compile the toolchain (unzip and gperf needed to compile toolchain, but aren't listed as dependencies in the README):

```
sudo apt update
sudo apt upgrade
sudo apt install curl flex bison libgmp3-dev libmpfr-dev autoconf build-essential libncurses5-dev libmpc-dev texinfo
sudo apt install gperf unzip
```

Install clang-format:
```
sudo apt install clang-format
```

Compile the toolchain (this will take a few hours, the tar steps can take a while and won't print anything to screen):

```
cd
git clone https://github.com/scanner-darkly/avr32-toolchain
cd avr32-toolchain
PREFIX=$HOME/avr32-tools make install-cross
```

Install ragel and compile the firmware:

```
sudo apt install ragel
export PATH="$HOME/avr32-tools/bin:$PATH"
cd
git clone --recursive https://github.com/samdoshi/teletype  # change to repo of your preference
cd teletype
cd module
make
```

### Building a firmware

Let's build and upload a copy of the [teletype][] firmware

First clone the repo, we need to clone recursively to bring in the `libavr32` submodule.
```
bash
git clone --recursive https://github.com/monome/teletype.git
cd teletype

```

Let's build it, we need to make sure that `avr32-gcc` and company are available on our path. Assuming they are installed in `~/avr32-tools`:
```
bash
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

### Serial port

Each of the modules has an unpopulated UART header on the rear, when populated it can be used with an FTDI cable or breakout board for print / trace debugging.

The following cables are known to work:

- [Sparkfun DEV-09718](https://www.sparkfun.com/products/9718)
- [FTDI TTL-232R-5V](http://www.ftdichip.com/Products/Cables/USBTTLSerial.htm) ([Octopart](https://octopart.com/ttl-232r-5v-ftdi-19172129))

When connecting, align the black cable with the `gnd` pin. Breakout boards are also available. 3.3V should work too.

Newer versions of OSX and Linux include builtin drivers for the FTDI cable. On OSX the simplest way to connect to the serial port is to use the `cu` program (type `~.` to quit), e.g.

```
bash
sudo cu -s 115200 -l <device>
```

On Linux and OSX, you can also use the `screen` command (type `C-a \`, or `C-a :quit` to quit), e.g.

```
bash
sudo screen <device> 115200
```

The value of `<device>` depends on the adaptor being used and the OS, try the following 2 commands to identify it:

```
bash
ls /dev | grep -i ttyusb   # should work on Linux
ls /dev | grep -i tty.usb  # should work on OSX
```


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
- [AVR32007: UC3 C-coding Guidelines for ARM7 Developers](http://www.atmel.com/Images/doc32075.pdf): very useful short overview of the UC3, even if you know nothing about ARM processors.
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
