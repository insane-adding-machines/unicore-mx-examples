##
## This file is part of the unicore-mx project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

PREFIX		?= arm-none-eabi
#PREFIX		?= arm-elf

TARGETS		:= stm32/f0 stm32/f1 stm32/f2 stm32/f3 stm32/f4 stm32/l0 stm32/l1
TARGETS		+= lpc/lpc13xx lpc/lpc17xx #lpc/lpc43xx
TARGETS		+= tiva/lm3s tiva/lm4f
TARGETS		+= efm32/efm32tg efm32/efm32g efm32/efm32lg efm32/efm32gg
TARGETS		+= vf6xx
TARGETS		+= qemu

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

UCMX_DIR ?= $(realpath unicore-mx)
EXAMPLE_RULES = elf

all: build

bin: EXAMPLE_RULES += bin
hex: EXAMPLE_RULES += hex
srec: EXAMPLE_RULES += srec
list: EXAMPLE_RULES += list
images: EXAMPLE_RULES += images

bin: build
hex: build
srec: build
list: build
images: build

build: lib examples

lib:
	$(Q)if [ ! "`ls -A $(UCMX_DIR)`" ] ; then \
		printf "######## ERROR ########\n"; \
		printf "\tunicore-mx is not initialized.\n"; \
		printf "\tPlease run:\n"; \
		printf "\t$$ git submodule init\n"; \
		printf "\t$$ git submodule update\n"; \
		printf "\tbefore running make.\n"; \
		printf "######## ERROR ########\n"; \
		exit 1; \
		fi
	$(Q)$(MAKE) -C $(UCMX_DIR)

EXAMPLE_DIRS:=$(sort $(dir $(wildcard $(addsuffix /*/*/Makefile,$(addprefix examples/,$(TARGETS))))))
$(EXAMPLE_DIRS): lib
	@printf "  BUILD   $@\n";
	$(Q)$(MAKE) --directory=$@ UCMX_DIR=$(UCMX_DIR) $(EXAMPLE_RULES)

examples: $(EXAMPLE_DIRS)
	$(Q)true

clean: $(EXAMPLE_DIRS:=.clean) styleclean
	$(Q)$(MAKE) -C unicore-mx clean

stylecheck: $(EXAMPLE_DIRS:=.stylecheck)
styleclean: $(EXAMPLE_DIRS:=.styleclean)


%.clean:
	$(Q)if [ -d $* ]; then \
		printf "  CLEAN   $*\n"; \
		$(MAKE) -C $* clean UCMX_DIR=$(UCMX_DIR) || exit $?; \
	fi;

%.styleclean:
	$(Q)$(MAKE) -C $* styleclean UCMX_DIR=$(UCMX_DIR)

%.stylecheck:
	$(Q)$(MAKE) -C $* stylecheck UCMX_DIR=$(UCMX_DIR)


.PHONY: build lib examples $(EXAMPLE_DIRS) install clean stylecheck styleclean \
        bin hex srec list images

