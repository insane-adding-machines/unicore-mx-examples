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

BINARY = cdcacm

VPATH += $(COMMON_DIR)/usb_cdcacm
CPPFLAGS += -I$(COMMON_DIR)/usb_cdcacm
OBJS = cdcacm-target.o

LDSCRIPT = ../lisa-m.ld

include ../../Makefile.include

