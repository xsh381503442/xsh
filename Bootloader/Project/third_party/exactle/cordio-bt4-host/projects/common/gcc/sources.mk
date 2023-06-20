###################################################################################################
#
# Source and include definition
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
#	WSF files
#--------------------------------------------------------------------------------------------------

C_FILES   += \
    $(ROOT_DIR)/ws-core/sw/wsf/common/wsf_buf.c \
    $(ROOT_DIR)/ws-core/sw/wsf/common/wsf_msg.c \
    $(ROOT_DIR)/ws-core/sw/wsf/common/wsf_queue.c \
    $(ROOT_DIR)/ws-core/sw/wsf/common/wsf_timer.c \
    $(ROOT_DIR)/ws-core/sw/wsf/generic/wsf_os.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/wsf/wsf_trace.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/wsf/wsf_assert.c \
    $(ROOT_DIR)/ws-core/sw/util/bda.c \
    $(ROOT_DIR)/ws-core/sw/util/bstream.c \
    $(ROOT_DIR)/ws-core/sw/util/calc128.c

INC_DIRS  += \
    $(ROOT_DIR)/ws-core/include \
    $(ROOT_DIR)/ws-core/sw/util \
    $(ROOT_DIR)/ws-core/sw/wsf/generic \
    $(ROOT_DIR)/ws-core/sw/wsf/include \
    $(ROOT_DIR)/cordio-bt4-host/platform

#--------------------------------------------------------------------------------------------------
# 	Project files
#--------------------------------------------------------------------------------------------------

INC_DIRS  += \
    $(ROOT_DIR)/cordio-bt4-host/projects/board/common \
    $(ROOT_DIR)/cordio-bt4-host/projects/common

C_FILES   += \
    $(ROOT_DIR)/cordio-bt4-host/projects/common/board.c \
    $(ROOT_DIR)/cordio-bt4-host/projects/common/main_time.c \
    $(ROOT_DIR)/cordio-bt4-host/projects/common/utils.c

#--------------------------------------------------------------------------------------------------
# 	Platform files
#--------------------------------------------------------------------------------------------------

INC_DIRS  += \
    $(ROOT_DIR)/cordio-bt4-host/platform \
    $(ROOT_DIR)/cordio-bt4-host/platform/cordio-bt4/inc \
    $(ROOT_DIR)/cordio-bt4-host/platform/cordio-bt4/img \
    $(ROOT_DIR)/cordio-bt4-host/platform/cordio-bt4/test/$(HOST_REV) \
    $(ROOT_DIR)/cordio-bt4-host/platform/CMSIS/Include \
    $(ROOT_DIR)/cordio-bt4-host/platform/CMSIS/Driver/Include \
    $(ROOT_DIR)/cordio-bt4-host/platform/CMSIS/Device/SMD/TC/Include

C_FILES   += \
    $(ROOT_DIR)/cordio-bt4-host/platform/CMSIS/Device/SMD/TC/Source/system_smd_tc2.c \
    $(ROOT_DIR)/cordio-bt4-host/platform/CMSIS/Device/SMD/TC/Source/GCC/startup_smd_tc2.c \
    $(ROOT_DIR)/cordio-bt4-host/platform/cordio_tc2.c \
    $(ROOT_DIR)/cordio-bt4-host/platform/platform.c

INC_DIRS  += \
    $(ROOT_DIR)/cordio-bt4-host/sw/llc/include

C_FILES   += \
    $(ROOT_DIR)/cordio-bt4-host/sw/llc/cordio-tc2/llc.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/llc/generic/llc_hci.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/llc/generic/llc_init.c
