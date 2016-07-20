#

# Include the common Makefile, which will also include the project specific
# config.mk file.

UNIT_TESTS = \
	unit/notes/test_notes.c

INTEGRATION_TESTS =

PRJ_ROOT = ..

INC_PATH = \
	src                                        \
	src/usb                                    \
	src/usb/ftdi                               \
	src/usb/hid                                \
	src/usb/midi                               \
	conf                                       \
	conf/trilogy                               \
	asf/avr32/boards                                       \
	asf/avr32/drivers/cpu/cycle_counter                    \
	asf/avr32/drivers/flashc                               \
	asf/avr32/drivers/gpio                                 \
	asf/avr32/drivers/intc                                 \
	asf/avr32/drivers/pm                                   \
	asf/avr32/drivers/spi                                  \
	asf/avr32/drivers/tc                                   \
	asf/avr32/drivers/twi                                  \
	asf/avr32/drivers/usart                                \
	asf/avr32/drivers/usbb                                 \
	asf/avr32/utils                                        \
	asf/avr32/utils/debug                                  \
	asf/avr32/utils/preprocessor                           \
	asf/common/boards                                      \
	asf/common/boards/user_board \
	asf/common/services/storage/ctrl_access                \
	asf/common/services/clock                              \
	asf/common/services/delay                              \
	asf/common/services/usb/ \
	asf/common/services/usb/uhc \
	asf/common/services/usb/class/msc                      \
	asf/common/services/usb/class/msc/host                 \
	asf/common/services/usb/class/hid                      \
	asf/common/services/spi/uc3_spi \
	asf/common/utils                                       


unity-dir = vendor/unity/src
unity-csrc = $(unity-dir)/unity.c

build-dir = ./build/


unit-exes-y := $(addprefix $(build-dir),$(basename $(UNIT_TESTS)))

clean-files += $(wildcard $(exes-y))

cflags-gnu-y += -Wall -Wstrict-prototypes #-Wmissing-prototypes
cppflags-gnu-y += $(foreach INC,$(addprefix $(PRJ_ROOT)/,$(INC_PATH)),-I$(INC))

cppflags-gnu-y += -I$(unity-dir) -D__TDD__

c_flags  = $(cppflags-gnu-y) $(cflags-gnu-y)

$(build-dir)%: %.c
	test -d $(dir $@) || mkdir -p $(dir $@)
	$(CC) $(c_flags) $< $(unity-csrc) -o $@


# default target
.PHONY: test
test: run-units

.PHONY: build-units
build-units: $(unit-exes-y)

.PHONY: run-units
run-units: $(unit-exes-y)
	@echo "\nrunning unit tests\n===================\n"
	@for exe in $(strip $(unit-exes-y)) ; do            \
		$$exe ;           	                        \
	done                                          \

.PHONY: clean
clean:
#echo $(clean-files)
#$(if $(strip $(clean-files)), $(RM) $(clean-files),)
	$(RM) -rf $(build-dir)

.PHONY: rebuild
rebuild: clean test

#.PHONY: exefiles
#exefiles: $(exes-y)

