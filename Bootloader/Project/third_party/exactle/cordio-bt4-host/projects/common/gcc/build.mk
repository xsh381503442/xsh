###################################################################################################
#
# bt4_2.0 build make targets
#
#         $Date: 2016-08-28 14:40:28 -0700 (Sun, 28 Aug 2016) $
#         $Revision: 8609 $
#  
# Copyright (c) 2015 ARM, Ltd., all rights reserved.
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

#--------------------------------------------------------------------------------------------------
#     Project
#--------------------------------------------------------------------------------------------------

# Parent makefile must export the following variables
#    CROSS_COMPILE
#    ROOT_DIR
#    LIBS
#    DEBUG
#    BIN
#    CPU
#    CFG_DEV

# Toolchain
CC         := $(CROSS_COMPILE)gcc
AR         := $(CROSS_COMPILE)ar
LD         := $(CROSS_COMPILE)gcc
DEP        := $(CROSS_COMPILE)gcc
OBJDUMP	   := $(CROSS_COMPILE)objdump
OBJCOPY	   := $(CROSS_COMPILE)objcopy
SIZE	   := $(CROSS_COMPILE)size
PACKIMG    := $(ROOT_DIR)/cordio-bt4-host/projects/pack_img.py
PYTHON     := python

#--------------------------------------------------------------------------------------------------
#     Configuration
#--------------------------------------------------------------------------------------------------

HOST_REV   ?= 2.2
HOST_TYPE  ?= ASIC
HOST_APP   ?= ble_ctr
FW_FILE    := $(ROOT_DIR)/cordio-bt4-host/platform/cordio-bt4/img/$(HOST_TYPE)-$(HOST_REV)-$(HOST_APP).img
HOST_REV_C := $(shell echo $(HOST_REV) | tr -d '.')

#--------------------------------------------------------------------------------------------------
#     Sources
#--------------------------------------------------------------------------------------------------

include sources*.mk

# Object file list
OBJ_FILES  := $(C_FILES:.c=.o)
OBJ_FILES  := $(subst $(ROOT_DIR)/,$(INT_DIR)/,$(OBJ_FILES))
OBJ_FILES  := $(subst ./,$(INT_DIR)/,$(OBJ_FILES))
DEP_FILES  := $(OBJ_FILES:.o=.d)
LD_SCR     := $(ROOT_DIR)/cordio-bt4-host/projects/common/cordio_tc-ram.ld

#--------------------------------------------------------------------------------------------------
#     Compilation flags
#--------------------------------------------------------------------------------------------------

# Hardware configuration
CFG_DEV    += CONFIG_HOST_REV=0x$(HOST_REV_C)
CFG_DEV    += CONFIG_HOST_TYPE_$(HOST_TYPE)=TRUE

# Debug
ifeq ($(TOKEN),0)
CFG_DEV    += WSF_TOKEN_ENABLED=FALSE
else
CFG_DEV    += WSF_TOKEN_ENABLED=TRUE
endif
ifeq ($(TRACE),0)
CFG_DEV    += WSF_TRACE_ENABLED=FALSE
else
CFG_DEV    += WSF_TRACE_ENABLED=TRUE
endif
ifeq ($(DEBUG),0)
CFG_DEV    += WSF_ASSERT_ENABLED=FALSE
else
CFG_DEV    += WSF_ASSERT_ENABLED=TRUE
endif

# Compiler flags
C_FLAGS    := -std=c99 -Os
C_FLAGS    += -Wall -Wextra -ffunction-sections -Wno-unused-parameter -Wshadow -Wundef
C_FLAGS    += -mcpu=cortex-m0plus -mthumb -march=armv6s-m -Wa,-mthumb 
C_FLAGS    += -mno-unaligned-access -fomit-frame-pointer -mapcs-frame -mabi=aapcs
C_FLAGS    += $(addprefix -I,$(INC_DIRS))
C_FLAGS    += $(addprefix -D,$(CFG_DEV))
C_FLAGS    += -DWSF_BUF_ALLOC_FAIL_ASSERT=FALSE

# Linker flags
LD_FLAGS   := -mthumb -march=armv6s-m -fwhole-program -nostartfiles -fno-exceptions
LD_FLAGS   += -Wl,-Map=$(BIN:.elf=.map)
LD_FLAGS   += -Wl,--gc-sections
##ENA_NANOLIB   : ##@build Enable (1) or disable (0, default) "nanolib"
ifeq ($(ENA_NANOLIB),1)
LD_FLAGS   += --specs=nano.specs
endif

# Dependency flags
DEP_FLAGS  := $(C_FLAGS) -MM -MF

#--------------------------------------------------------------------------------------------------
#     Outputs
#--------------------------------------------------------------------------------------------------

OBJ        := $(BIN:.elf=.bin)
IMG        := $(BIN:.elf=.img)
SPF        := $(BIN:.elf=.spf)

#--------------------------------------------------------------------------------------------------
#     Targets
#--------------------------------------------------------------------------------------------------

ifeq ($(TOKEN),1)
all: img $(SPF) token
else
all: img $(SPF)
endif

img: $(LIBS) $(BIN) $(OBJ) $(IMG)
	@$(OBJCOPY) -O binary $(BIN) $(BIN:.elf=.bin)
	@$(OBJCOPY) -O ihex $(BIN) $(BIN:.elf=.hex)
	@$(OBJDUMP) -t $(BIN) > $(BIN:.elf=.sym)
	@echo "+++ Build options"
	@echo "    DEBUG   = $(DEBUG)"
	@echo "    TOKEN   = $(TOKEN)"
	@echo "    CFG_DEV = $(CFG_DEV)"
	@echo "+++ Object summary: $(INT_DIR)"
	@-find $(INT_DIR) -name *.o | xargs $(SIZE) -t	
	@echo "+++ Binary summary: $(BIN)"
	@-$(SIZE) $(BIN)

$(BIN): $(OBJ_FILES) $(LIBS)
	@echo "+++ Linking: $@"
	@mkdir -p $(BIN_DIR)
	@$(LD) -o $(BIN) $(LD_FLAGS) $(OBJ_FILES) $(LIBS) -T$(LD_SCR)
	@$(OBJDUMP) -t $(BIN) > $(BIN:.elf=.sym)

$(OBJ): $(BIN)
	@echo "+++ Extracting: $@"
	@mkdir -p $(dir $@)
	@$(OBJCOPY) -O binary $< --remove-section .note.gnu.build-id $@

$(IMG): $(OBJ)
	@echo "+++ Packing: $@"
	@mkdir -p $(dir $@)
	@$(PYTHON) $(PACKIMG) $< $@ old

$(SPF): $(IMG)
	@echo "+++ Packing: $@"
	@mkdir -p $(dir $@)
	@cat $< $(FW_FILE) > $@
	@echo -e "\x00\x00\x00\x00" >> $@

$(INT_DIR)/%.o: $(ROOT_DIR)/%.c
	@echo "+++ Compiling: $<"
	@mkdir -p $(dir $@)
	@$(CC) $(C_FLAGS) -DMODULE_ID=$(call FILE_HASH,$<) -c -o $@ $<
	@$(if $(DEP),$(DEP) $(DEP_FLAGS) $(subst .o,.d,$@) -MP -MT $@ $<,)

clean: token.clean
	@rm -rf $(INT_DIR)
	@rm -rf $(BIN_DIR)

include $(ROOT_DIR)/ws-core/projects/common/gcc/token.mk
-include $(DEP_FILES)

.PHONY: all $(LIBS) clean
