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
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_app_slave.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_hci.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_sec.mk
include $(ROOT_DIR)/cordio-bt4-host/projects/common/gcc/sources_stack.mk

#--------------------------------------------------------------------------------------------------
# 	Meds app files
#--------------------------------------------------------------------------------------------------

C_FILES += \
    $(ROOT_DIR)/sw/apps/meds/meds_main.c \
    $(ROOT_DIR)/sw/apps/meds/meds_blp.c \
    $(ROOT_DIR)/sw/apps/meds/meds_htp.c \
    $(ROOT_DIR)/sw/apps/meds/meds_wsp.c \
    $(ROOT_DIR)/sw/apps/meds/meds_glp.c \
    $(ROOT_DIR)/sw/apps/meds/meds_plx.c \
    $(ROOT_DIR)/sw/profiles/blps/blps_main.c \
    $(ROOT_DIR)/sw/profiles/htps/htps_main.c \
    $(ROOT_DIR)/sw/profiles/wsps/wsps_main.c \
    $(ROOT_DIR)/sw/profiles/plxps/plxps_main.c \
    $(ROOT_DIR)/sw/profiles/plxps/plxps_db.c \
    $(ROOT_DIR)/sw/profiles/glps/glps_main.c \
    $(ROOT_DIR)/sw/profiles/glps/glps_db.c \
    $(ROOT_DIR)/sw/services/svc_bps.c \
    $(ROOT_DIR)/sw/services/svc_dis.c \
    $(ROOT_DIR)/sw/services/svc_hts.c \
    $(ROOT_DIR)/sw/services/svc_plxs.c \
    $(ROOT_DIR)/sw/services/svc_gls.c \
    $(ROOT_DIR)/sw/services/svc_wp.c \
    $(ROOT_DIR)/sw/services/svc_wss.c

INC_DIRS += \
    $(ROOT_DIR)/sw/apps/meds \
    $(ROOT_DIR)/sw/profiles/blps \
    $(ROOT_DIR)/sw/profiles/htps \
    $(ROOT_DIR)/sw/profiles/plxps \
    $(ROOT_DIR)/sw/profiles/glps \
    $(ROOT_DIR)/sw/profiles/wsps

#--------------------------------------------------------------------------------------------------
# 	Startup files
#--------------------------------------------------------------------------------------------------

C_FILES   += \
    $(ROOT_DIR)/cordio-bt4-host/platform/board_flash.c \
    $(ROOT_DIR)/cordio-bt4-host/projects/meds/main.c
