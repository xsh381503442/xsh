###################################################################################################
#
# Makefile for building tool
# 
#         $Date: 2016-07-21 14:00:47 -0400 (Thu, 21 Jul 2016) $
#         $Revision: 7919 $
#  
# Copyright (c) 2016 ARM, Ltd., all rights reserved.
# ARM confidential and proprietary.
#
# IMPORTANT.  Your use of this file is governed by a Software License Agreement
# ("Agreement") that must be accepted in order to download or otherwise receive a
# copy of this file.  You may not use or copy this file for any purpose other than
# as described in the Agreement.  If you do not agree to all of the terms of the
# Agreement do not use this file and delete all copies in your possession or control;
# if you do not have a copy of the Agreement, you must contact ARM, Ltd. prior
# to any use, copying or further distribution of this software.
#
###################################################################################################

HOSTCC := gcc
HOSTCPPFLAGS += -Wall -Wextra -Wno-char-subscripts

FWUPDATER := fwupdater

Q=@
E=echo
ifeq ($(V), 1)
Q=
E=true
endif

all: $(FWUPDATER)

.PHONY: clean

$(FWUPDATER): ../fwupdater.c ../serial_linux.c
	@$(E) "  HOSTCC   " $@
	$(Q)$(HOSTCC) -Os -o $@ -I.. -I../../../../ws-core/sw/util -I../../../../ws-core/sw/wsf/generic $^
	
clean:
	@$(E) "  CLEAN    "
	$(Q)$(RM) -f $(FWUPDATER)
