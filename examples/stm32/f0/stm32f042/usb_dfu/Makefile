##
## This file is part of the unicore-mx project.
##
## Copyright (C) 2018 Ariel D'Alessandro <ariel@vanguardiasur.com.ar>
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

BINARY = usbdfu

VPATH += $(COMMON_DIR)/usb_dfu
CPPFLAGS += -I$(COMMON_DIR)/usb_dfu
OBJS = usbdfu-target.o

LDSCRIPT = stm32f042f6p6_usb_dfu.ld

include ../../Makefile.include

