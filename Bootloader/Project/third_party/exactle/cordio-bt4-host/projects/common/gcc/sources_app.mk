###################################################################################################
#
# Source and include definition
#
#         $Date: 2016-08-19 04:58:31 -0700 (Fri, 19 Aug 2016) $
#         $Revision: 8404 $
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
#	Common app files
#--------------------------------------------------------------------------------------------------

INC_DIRS += \
    $(ROOT_DIR)/sw/apps/app \
    $(ROOT_DIR)/sw/apps/app/include

C_FILES += \
    $(ROOT_DIR)/sw/apps/app/app_main.c \
    $(ROOT_DIR)/sw/apps/app/app_server.c \
    $(ROOT_DIR)/sw/apps/app/app_terminal.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/app/app_param.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/app/app_ui.c \
    $(ROOT_DIR)/sw/apps/app/generic/app_db.c \
    $(ROOT_DIR)/sw/apps/app/generic/app_hw.c
