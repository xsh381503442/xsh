###################################################################################################
#
# Makefile for building the cycling sample application
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

# GCC ARM cross toolchain
CROSS_COMPILE := arm-none-eabi-
CPU        := cortex-m0

# Inputs
ROOT_DIR   := ../../../..

# Output
INT_DIR    := obj-$(CPU)
BIN_DIR    := bin-$(CPU)
BIN        := $(BIN_DIR)/cycling-obj.elf

# Options
DEBUG      := 0
TOKEN      := 0
TRACE      := 1
USE_SRC    := 1
CFG_DEV    := WSF_MS_PER_TICK=10
CFG_DEV    += WSF_PRINTF_MAX_LEN=128
CFG_DEV    += WSF_MAX_HANDLERS=20
CFG_DEV    += INIT_FLASH=1 INIT_WAKEUP=1 INIT_GPO=1
LIBS       :=
ifeq ($(USE_SRC),0)
LIBS       := $(ROOT_DIR)/projects/generic/stacklib/gcc/bin-$(CPU)/libstacklib.a
endif

ALLOW_DEEP_SLEEP ?= FALSE
CFG_DEV          += CONFIG_ALLOW_DEEP_SLEEP=$(ALLOW_DEEP_SLEEP)

ALLOW_SETTING_WRITE ?= TRUE
CFG_DEV             += CONFIG_ALLOW_SETTING_WRITE=$(ALLOW_SETTING_WRITE)

#--------------------------------------------------------------------------------------------------
#     Targets
#--------------------------------------------------------------------------------------------------

ifeq ($(USE_SRC),1)
include $(ROOT_DIR)/projects/generic/stacklib/gcc/sources*.mk
endif
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/build.mk
