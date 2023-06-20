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

include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_app.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_app_master.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_hci.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_sec.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_stack.mk

#--------------------------------------------------------------------------------------------------
# 	Medc app files
#--------------------------------------------------------------------------------------------------

C_FILES += \
    $(ROOT_DIR)/sw/apps/medc/medc_main.c \
    $(ROOT_DIR)/sw/apps/medc/medc_blp.c \
    $(ROOT_DIR)/sw/apps/medc/medc_glp.c \
    $(ROOT_DIR)/sw/apps/medc/medc_hrp.c \
    $(ROOT_DIR)/sw/apps/medc/medc_htp.c \
    $(ROOT_DIR)/sw/apps/medc/medc_wsp.c \
    $(ROOT_DIR)/sw/apps/medc/medc_plx.c \
    $(ROOT_DIR)/sw/profiles/blpc/blpc_main.c \
    $(ROOT_DIR)/sw/profiles/dis/dis_main.c \
    $(ROOT_DIR)/sw/profiles/gatt/gatt_main.c \
    $(ROOT_DIR)/sw/profiles/glpc/glpc_main.c \
    $(ROOT_DIR)/sw/profiles/hrpc/hrpc_main.c \
    $(ROOT_DIR)/sw/profiles/htpc/htpc_main.c \
    $(ROOT_DIR)/sw/profiles/plxpc/plxpc_main.c \
    $(ROOT_DIR)/sw/profiles/wspc/wspc_main.c

INC_DIRS += \
    $(ROOT_DIR)/sw/apps/medc \
    $(ROOT_DIR)/sw/profiles/blpc \
    $(ROOT_DIR)/sw/profiles/dis \
    $(ROOT_DIR)/sw/profiles/gatt \
    $(ROOT_DIR)/sw/profiles/glpc \
    $(ROOT_DIR)/sw/profiles/hrpc \
    $(ROOT_DIR)/sw/profiles/htpc \
    $(ROOT_DIR)/sw/profiles/plxpc \
    $(ROOT_DIR)/sw/profiles/wspc

#--------------------------------------------------------------------------------------------------
# 	Startup files
#--------------------------------------------------------------------------------------------------

C_FILES   += \
    $(ROOT_DIR)/cordio-bt4-host/platform/board_flash.c \
    $(ROOT_DIR)/cordio-bt4-host/projects/medc/main.c
