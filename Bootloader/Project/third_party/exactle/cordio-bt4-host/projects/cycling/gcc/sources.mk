###################################################################################################
#
# Source and include definition
#  
#         $Date: 2016-08-28 14:40:28 -0700 (Sun, 28 Aug 2016) $
#         $Revision: 8609 $
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

include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_app.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_app_slave.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_hci.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_sec.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_stack.mk

#--------------------------------------------------------------------------------------------------
# 	Fit app files
#--------------------------------------------------------------------------------------------------

C_FILES += \
    $(ROOT_DIR)/sw/apps/cycling/cycling_main.c \
    $(ROOT_DIR)/sw/profiles/bas/bas_main.c \
    $(ROOT_DIR)/sw/profiles/cpp/cpps_main.c \
    $(ROOT_DIR)/sw/profiles/cscp/cscps_main.c \
    $(ROOT_DIR)/sw/services/svc_batt.c \
    $(ROOT_DIR)/sw/services/svc_cps.c \
    $(ROOT_DIR)/sw/services/svc_cscs.c \
    $(ROOT_DIR)/sw/services/svc_dis.c

INC_DIRS += \
    $(ROOT_DIR)/sw/apps/cycling \
    $(ROOT_DIR)/sw/profiles/bas \
    $(ROOT_DIR)/sw/profiles/cpp \
    $(ROOT_DIR)/sw/profiles/cscp \
    $(ROOT_DIR)/sw/profiles/cycling

#--------------------------------------------------------------------------------------------------
# 	Startup files
#--------------------------------------------------------------------------------------------------

C_FILES   += \
    $(ROOT_DIR)/cordio-bt4-host/platform/board_flash.c \
    $(ROOT_DIR)/cordio-bt4-host/projects/cycling/main.c