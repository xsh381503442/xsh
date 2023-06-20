###################################################################################################
#
# Makefile for building the exactLE library
# 
#         $Date: 2016-07-21 21:47:38 -0700 (Thu, 21 Jul 2016) $
#         $Revision: 7936 $
#  
# Copyright (c) 2012 Wicentric, Inc., all rights reserved.
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

# GCC ARM cross toolchain
CROSS_COMPILE := arm-none-eabi-
CPU        := cortex-m0

# Toolchain
ifneq (,$(CROSS_COMPILE))
CC         := $(CROSS_COMPILE)gcc
endif
AR         := $(CROSS_COMPILE)ar
LD         := $(CROSS_COMPILE)ld
DEP        := $(CROSS_COMPILE)gcc
SIZE       := $(CROSS_COMPILE)size

# Inputs
ROOT_DIR   := $(shell pwd)/../../../..

# Output
INT_DIR    := obj-$(CPU)
BIN_DIR    := bin-$(CPU)
BIN        := $(BIN_DIR)/libstacklib.a

#--------------------------------------------------------------------------------------------------
#     Sources
#--------------------------------------------------------------------------------------------------

INC_DIRS   :=
C_FILES    :=
include sources*.mk

LIB_DIRS   :=
LIB_FILES  :=

# Object file list
OBJ_FILES  := $(C_FILES:.c=.o)
OBJ_FILES  := $(subst $(ROOT_DIR)/,$(INT_DIR)/,$(OBJ_FILES))
DEP_FILES  := $(OBJ_FILES:.o=.d)

#--------------------------------------------------------------------------------------------------
#     Compilation flags
#--------------------------------------------------------------------------------------------------

# Compiler flags
C_FLAGS    := -std=c99
C_FLAGS    += -Wall #-pedantic #-Werror
C_FLAGS    += $(addprefix -I,$(INC_DIRS))
ifeq (arm-,$(findstring arm-,$(CROSS_COMPILE)))
C_FLAGS    += -mcpu=$(CPU) -mthumb -mlittle-endian
endif
ifeq ($(DEBUG),1)
C_FLAGS    += -O0 -g
C_FLAGS    += -DWSF_ASSERT_ENABLED=TRUE
C_FLAGS    += -DWSF_BUF_STATS=TRUE
else
C_FLAGS    += -Os -fno-common -fomit-frame-pointer
endif
ifeq ($(TRACE),1)
C_FLAGS    += -DWSF_TRACE_ENABLED=TRUE
endif
C_FLAGS    += $(addprefix -D,$(CFG_STACK))

# Dependency flags
DEP_FLAGS  := $(C_FLAGS) -MM -MF

#--------------------------------------------------------------------------------------------------
#     Targets
#--------------------------------------------------------------------------------------------------

all: $(BIN)
	@echo "+++ Binary summary: $(BIN)"
	@-$(SIZE) -t $(BIN)

$(BIN): $(OBJ_FILES)
	@echo "+++ Archiving: $@"
	@mkdir -p $(BIN_DIR)
	@$(AR) rcs $(BIN) $(OBJ_FILES)

$(INT_DIR)/%.o: $(ROOT_DIR)/%.c
	@echo "+++ Compiling: $<"
	@mkdir -p $(dir $@)
	@$(CC) $(C_FLAGS) -DMODULE_ID=$(call FILE_HASH,$<) -c -o $@ $<
	@$(if $(DEP),$(DEP) $(DEP_FLAGS) $(subst .o,.d,$@) -MP -MT $@ $<,)

clean: token.clean
	@rm -rf $(INT_DIR)
	@rm -rf $(BIN_DIR)

-include $(ROOT_DIR)/ws-core/projects/common/gcc/token.mk
-include $(DEP_FILES)

.PHONY: clean
