###################################################################################################
#
# Token generation make targets
# 
#         $Date: 2015-10-05 09:54:16 -0700 (Mon, 05 Oct 2015) $
#         $Revision: 4112 $
#  
# Copyright (c) 2013 Wicentric, Inc., all rights reserved.
# Wicentric confidential and proprietary.
#
# IMPORTANT.  Your use of this file is governed by a Software License Agreement
# ("Agreement") that must be accepted in order to download or otherwise receive a
# copy of this file.  You may not use or copy this file for any purpose other than
# as described in the Agreement.  If you do not agree to all of the terms of the
# Agreement do not use this file and delete all copies in your possession or control;
# if you do not have a copy of the Agreement, you must contact Wicentric, Inc. prior
# to any use, copying or further distribution of this software.
#
###################################################################################################

#--------------------------------------------------------------------------------------------------
#     Project
#--------------------------------------------------------------------------------------------------

# Parent makefile must export the following variables
#    CC
#    ROOT_DIR
#    BIN
#    C_FILES
#    C_FLAGS

# Toolchain
PYTHON     := python
TMON       := $(PYTHON) $(ROOT_DIR)/tools/trace/trace_monitor.py

# Input
ifeq ($(shell uname),Linux)
TRACE_DEV  := /dev/ttyACM0
else
TRACE_DEV  := COM3
endif

# Output
TOK_DIR    := tok
TOKEN_FILE := $(BIN:.a=.tokens)
TOKEN_FILE := $(TOKEN_FILE:.elf=.tokens)

#-------------------------------------------------------------------------------
#     Scripts
#-------------------------------------------------------------------------------

ifeq ($(TOKEN),1)
HASH_SCR   := import sys, binascii; \
              print hex(binascii.crc32(sys.argv[1]) & 0xFFFF)
FILE_HASH   = $(shell $(PYTHON) -c "$(HASH_SCR)" $(notdir $(1)))
else
FILE_HASH   =
endif

#--------------------------------------------------------------------------------------------------
#     Objects
#--------------------------------------------------------------------------------------------------

ifeq ($(TOKEN),1)
TOK_FILES  := $(C_FILES:.c=.pp)
TOK_FILES  := $(subst $(ROOT_DIR)/,$(TOK_DIR)/,$(TOK_FILES))
endif

#--------------------------------------------------------------------------------------------------
#     Compilation flags
#--------------------------------------------------------------------------------------------------

ifeq ($(TOKEN),1)
C_FLAGS    += -DWSF_TOKEN_ENABLED=TRUE
endif

#--------------------------------------------------------------------------------------------------
#     Targets
#--------------------------------------------------------------------------------------------------

token: $(TOK_FILES)
	@rm -f $(TOKEN_FILE)
	@mkdir -p $(dir $(TOKEN_FILE))
	@find $(TOK_DIR) -name \*.mod -exec cat {} \; >> $(TOKEN_FILE)
	@find $(TOK_DIR) -name \*.pp -exec grep __WSF_TOKEN_DEFINE__ {} \; | cut -d"(" -f2 | cut -d")" -f1 >> $(TOKEN_FILE)

$(TOK_DIR)/%.pp: $(ROOT_DIR)/%.c
	@echo "+++ Scanning: $<"
	@mkdir -p $(dir $@)
	@$(CC) $(C_FLAGS) -DTOKEN_GENERATION -DMODULE_ID=$(call FILE_HASH,$<) -E -o $@ $<
	@echo "$(call FILE_HASH,$<), 0, $<, , ," >> $(@:.pp=.mod)

token.clean:
	@rm -rf $(TOK_DIR)
	@rm -f $(TOKEN_FILE)

token.monitor:
	@$(TMON) $(TOKEN_FILE) $(TRACE_DEV) $(TRACE_FILTER)

.PHONY: token.clean token.monitor
