#

PRJ_ROOT = ..

INC_PATH = \
	src                                        \
	src/usb                                    \
	src/usb/ftdi                               \
	src/usb/hid                                \
	src/usb/midi                               \
	conf                                       \
	conf/trilogy                               \
	asf/avr32/boards                           \
	asf/avr32/drivers/cpu/cycle_counter        \
	asf/avr32/drivers/flashc                   \
	asf/avr32/drivers/gpio                     \
	asf/avr32/drivers/intc                     \
	asf/avr32/drivers/pm                       \
	asf/avr32/drivers/spi                      \
	asf/avr32/drivers/tc                       \
	asf/avr32/drivers/twi                      \
	asf/avr32/drivers/usart                    \
	asf/avr32/drivers/usbb                     \
	asf/avr32/utils                            \
	asf/avr32/utils/debug                      \
	asf/avr32/utils/preprocessor               \
	asf/common/boards                          \
	asf/common/boards/user_board               \
	asf/common/services/storage/ctrl_access    \
	asf/common/services/clock                  \
	asf/common/services/delay                  \
	asf/common/services/usb                    \
	asf/common/services/usb/uhc                \
	asf/common/services/usb/class/msc          \
	asf/common/services/usb/class/msc/host     \
	asf/common/services/usb/class/hid          \
	asf/common/services/spi/uc3_spi            \
	asf/common/utils

SRC_DIR = $(PRJ_ROOT)/src/

UNIT_DIR = unit

INTEGRATION_DIR = integration

CC = gcc
LINK = $(CC)

CFLAGS = -Wall -Wstrict-prototypes #-Wmissing-prototypes

TARGET_EXTENSION = out


MSG_COMPILE = "CC      < $<"
MSG_LINK    = "LN      > $@"
MSG_TEST    = "RUN     > $@"
MSG_CLEAN   = "CLEAN   $<"

unit-tests = $(shell find $(UNIT_DIR) -type f -name 'test_*.c')
#integration-tests = $(shell find $(INTEGRATION_DIR) -type f -name 'test_*.c')
tests = $(unit-tests) #$(integration-tests)

build-dir = build/

unity-dir = vendor/unity/src/
unity-csrc = $(wildcard $(unity-dir)*.c)
unity-objs = $(patsubst $(unity-dir)%.c,$(build-dir)%.o,$(unity-csrc))

test-objs = $(addprefix $(build-dir), $(addsuffix .o,$(basename $(tests))))
test-deps = $(addprefix $(build-dir), $(addsuffix .d,$(basename $(tests))))
test-execs = $(addprefix $(build-dir), $(addsuffix .$(TARGET_EXTENSION),$(basename $(tests))))
test-results = $(addprefix $(build-dir), $(addsuffix .txt,$(basename $(tests))))

test-include-dir = include/

cflags-inc = $(foreach INC,$(addprefix $(PRJ_ROOT)/,$(INC_PATH)),-I$(INC))
cppflags = -DTEST -I$(unity-dir) -I$(test-include-dir)

#
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
#
depflags = -MT $@ -MMD -MP -MF $(build-dir)$*.Td

COMPILE = $(CC) $(depflags) $(CFLAGS) $(cppflags) $(cflags-inc) -c

POSTCOMPILE = mv -f $(build-dir)$*.Td $(build-dir)$*.d

# hide command output by default, but allow the user to override this
# by adding V=1 on the command line.
ifdef V
  ifeq ("$(origin V)", "command line")
    VERBOSE = $(V)
  endif
endif
ifndef VERBOSE
  VERBOSE = 0
endif

ifeq ($(VERBOSE), 1)
  Q =
else
  Q = @
endif


# compile; src code
$(build-dir)%.o:: $(SRC_DIR)%.c $(build-dir)%.d
	@echo $(MSG_COMPILE)
	$(Q)test -d $(dir $@) || mkdir -p $(dir $@)
	$(Q)$(COMPILE) $< -o $@
	$(Q)$(POSTCOMPILE)

# compile; test code
$(build-dir)%.o:: %.c $(build-dir)%.d
	@echo $(MSG_COMPILE)
	$(Q)test -d $(dir $@) || mkdir -p $(dir $@)
	$(Q)$(COMPILE) $< -o $@
	$(Q)$(POSTCOMPILE)

# compile; unity code
$(build-dir)%.o:: $(unity-dir)%.c $(unity-dir)%.h
	@echo $(MSG_COMPILE)
	$(Q)test -d $(dir $@) || mkdir -p $(dir $@)
	$(Q)$(COMPILE) $< -o $@
	$(Q)$(POSTCOMPILE)

# link; test code
$(build-dir)%.$(TARGET_EXTENSION): $(build-dir)%.o $(unity-objs)
	@echo $(MSG_LINK)
	$(Q)$(LINK) -o $@ $^

# run test execs and capture results
$(build-dir)%.txt: $(build-dir)%.$(TARGET_EXTENSION)
	@echo $(MSG_TEST)
	$(Q)-./$< > $@ 2>&1


# default target

test: $(test-results)
	@echo "-----------------------\nIGNORES:\n-----------------------"
	@echo `grep --no-filename -s IGNORE $(test-results)`
	@echo "-----------------------\nFAILURES:\n-----------------------"
	@echo `grep --no-filename -s FAIL $(test-results)`
	@echo "\nDONE"

clean:
	$(RM) -rf $(build-dir)

rebuild: clean test


.PHONY: test
.PHONY: rebuild
.PHONY: clean

.PRECIOUS: $(unity-objs)
.PRECIOUS: $(test-objs)
.PRECIOUS: $(test-execs)
.PRECIOUS: $(test-results)


$(build-dir)%.d: ;
.PRECIOUS: $(test-deps)

# FIXME: this misses dependencies for the files under src/ ???
-include $(test-deps)
