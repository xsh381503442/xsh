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
#	HCI files
#--------------------------------------------------------------------------------------------------

INC_DIRS += \
    $(ROOT_DIR)/sw/hci/dual_chip \
    $(ROOT_DIR)/sw/hci/include \
    $(ROOT_DIR)/cordio-bt4-host/sw/hci

C_FILES += \
    $(ROOT_DIR)/sw/hci/generic/hci_core.c \
    $(ROOT_DIR)/sw/hci/dual_chip/hci_cmd.c \
    $(ROOT_DIR)/sw/hci/dual_chip/hci_core_ps.c \
    $(ROOT_DIR)/sw/hci/dual_chip/hci_evt.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/hci/hci_vs.c \
    $(ROOT_DIR)/cordio-bt4-host/sw/hci/hci_tr.c
