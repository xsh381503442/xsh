###################################################################################################
#
# Utility to pack a binary file to a Cordio image and SPF.
#
#         $Date: 2015-10-25 14:34:09 -0700 (Sun, 25 Oct 2015) $
#         $Revision: 4307 $
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

import sys, os
import struct, time, subprocess
import binascii

DEBUG = False

if __name__ == '__main__':
    if len(sys.argv) != 6:
        print "USAGE: %s input.bin input.spf output.img output.spf [new|old]" % sys.argv[0]
        exit()

    if (sys.argv[5] != "new") and (sys.argv[5] != "old"):
        print "USAGE: %s input.bin input.spf output.img output.spf [new|old]" % sys.argv[0]
        exit()
    new_style = (sys.argv[3] == "new")

    in_file_name  = sys.argv[1]
    in_img_name   = sys.argv[2]
    out_file_name = sys.argv[3]
    out_spf_name  = sys.argv[4]

    in_file = open(in_file_name, 'rb')
    if in_file == None:
        print "ERROR: cannot open input file" % (in_file_name)
        exit()

    in_img = open(in_img_name, 'rb')
    if in_img == None:
        print "ERROR: cannot open input file" % (in_img_name)
        exit()

    out_file = open(out_file_name, 'wb')
    if out_file == None:
        print "ERROR: cannot open output file" % (out_file_name)
        exit()
        
    out_spf = open(out_spf_name, 'wb')
    if out_spf == None:
        print "ERROR: cannot open output file" % (out_spf_name)
        exit()

    # Read binary
    temp = in_file.read()

    # Calculate CRC
    CRC32_INIT_VAL = 0xFFFFFFFF
    crc = CRC32_INIT_VAL
    if new_style:
        tempc = ""
        tempc += struct.pack("<I", len(temp))
        crc = binascii.crc32(tempc, crc) & 0xFFFFFFFF  # data_eln
        tempc = ""
        tempc += struct.pack("<I", 0)
        crc = binascii.crc32(tempc, crc) & 0xFFFFFFFF  # offset
        crc = binascii.crc32(tempc, crc) & 0xFFFFFFFF  # entry
    crc = binascii.crc32(temp, crc) & 0xFFFFFFFF

    # Create header
    SWIMAGE_HEADER_COOKIE_V0 = 0x30444D53
    SWIMAGE_TYPE_HOSTFWIMAGE = 0
    hdr  = struct.pack("<I", SWIMAGE_HEADER_COOKIE_V0)  # cookie
    hdr += struct.pack("<B", 24)                        # hdr_len
    hdr += struct.pack("<B", SWIMAGE_TYPE_HOSTFWIMAGE)  # type
    hdr += struct.pack("<B", 0)                         # rev_minor
    hdr += struct.pack("<B", 0)                         # rev_major
    hdr += struct.pack("<I", len(temp))                 # data_len
    hdr += struct.pack("<I", 0)                         # offset
    hdr += struct.pack("<I", 0x20000000)                # entry
    hdr += struct.pack("<I", crc)                       # crc32

    # Write data to file
    out_file.write(hdr)
    out_file.write(temp)

    out_spf.write(hdr)
    out_spf.write(temp)
    temp = in_img.read()
    out_spf.write(temp)
    out_spf.write(struct.pack("<I", 0))
    
    in_file.close()
    in_img.close()
    out_file.close()
    out_spf.close()
    