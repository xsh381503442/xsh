###################################################################################################
#
# Source and include definition
# 
#         $Date: 2016-08-22 17:32:42 -0700 (Mon, 22 Aug 2016) $
#         $Revision: 8489 $
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

INC_DIRS += \
	$(ROOT_DIR)/ws-core/include \
	$(ROOT_DIR)/sw/stack/att \
	$(ROOT_DIR)/sw/stack/cfg \
	$(ROOT_DIR)/sw/stack/dm \
	$(ROOT_DIR)/sw/stack/hci \
	$(ROOT_DIR)/sw/stack/include \
	$(ROOT_DIR)/sw/stack/l2c \
	$(ROOT_DIR)/sw/stack/smp \
	$(ROOT_DIR)/sw/sec/include \
	$(ROOT_DIR)/ws-core/sw/util \
	$(ROOT_DIR)/ws-core/sw/wsf/generic \
	$(ROOT_DIR)/ws-core/sw/wsf/include

C_FILES += \
	$(ROOT_DIR)/sw/stack/att/att_main.c \
	$(ROOT_DIR)/sw/stack/att/att_uuid.c \
	$(ROOT_DIR)/sw/stack/att/attc_disc.c \
	$(ROOT_DIR)/sw/stack/att/attc_main.c \
	$(ROOT_DIR)/sw/stack/att/attc_proc.c \
	$(ROOT_DIR)/sw/stack/att/attc_read.c \
	$(ROOT_DIR)/sw/stack/att/attc_sign.c \
	$(ROOT_DIR)/sw/stack/att/attc_write.c \
	$(ROOT_DIR)/sw/stack/att/atts_ccc.c \
	$(ROOT_DIR)/sw/stack/att/atts_ind.c \
	$(ROOT_DIR)/sw/stack/att/atts_main.c \
	$(ROOT_DIR)/sw/stack/att/atts_proc.c \
	$(ROOT_DIR)/sw/stack/att/atts_read.c \
	$(ROOT_DIR)/sw/stack/att/atts_sign.c \
	$(ROOT_DIR)/sw/stack/att/atts_write.c \
	$(ROOT_DIR)/sw/stack/dm/dm_adv.c \
	$(ROOT_DIR)/sw/stack/dm/dm_adv_leg.c \
	$(ROOT_DIR)/sw/stack/dm/dm_adv_priv.c \
	$(ROOT_DIR)/sw/stack/dm/dm_conn.c \
	$(ROOT_DIR)/sw/stack/dm/dm_conn_slave.c \
	$(ROOT_DIR)/sw/stack/dm/dm_conn_slave_leg.c \
	$(ROOT_DIR)/sw/stack/dm/dm_conn_sm.c \
	$(ROOT_DIR)/sw/stack/dm/dm_dev.c \
	$(ROOT_DIR)/sw/stack/dm/dm_main.c \
	$(ROOT_DIR)/sw/stack/dm/dm_priv.c \
	$(ROOT_DIR)/sw/stack/dm/dm_scan.c \
	$(ROOT_DIR)/sw/stack/dm/dm_scan_leg.c \
	$(ROOT_DIR)/sw/stack/dm/dm_sec.c \
	$(ROOT_DIR)/sw/stack/dm/dm_sec_lesc.c \
	$(ROOT_DIR)/sw/stack/dm/dm_sec_slave.c \
	$(ROOT_DIR)/sw/stack/l2c/l2c_coc.c \
	$(ROOT_DIR)/sw/stack/l2c/l2c_main.c \
	$(ROOT_DIR)/sw/stack/l2c/l2c_slave.c \
	$(ROOT_DIR)/sw/stack/smp/smp_act.c \
	$(ROOT_DIR)/sw/stack/smp/smp_main.c \
	$(ROOT_DIR)/sw/stack/smp/smp_non.c \
	$(ROOT_DIR)/sw/stack/smp/smp_sc_main.c \
	$(ROOT_DIR)/sw/stack/smp/smp_sc_act.c \
	$(ROOT_DIR)/sw/stack/smp/smpi_act.c \
	$(ROOT_DIR)/sw/stack/smp/smpi_sc_act.c \
	$(ROOT_DIR)/sw/stack/smp/smpi_sc_sm.c \
	$(ROOT_DIR)/sw/stack/smp/smpi_sm.c \
	$(ROOT_DIR)/sw/stack/smp/smpr_act.c \
	$(ROOT_DIR)/sw/stack/smp/smpr_sc_act.c \
	$(ROOT_DIR)/sw/stack/smp/smpr_sc_sm.c \
	$(ROOT_DIR)/sw/stack/smp/smpr_sm.c
