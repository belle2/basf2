#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM time conversion payloads.

import sys
import basf2
from ROOT.Belle2 import KLMDatabaseImporter, KLMTimeConversion

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

dbImporter = KLMDatabaseImporter()

timeConversion = KLMTimeConversion()
# Time offset in ns.
timeConversion.setTimeOffset(0)
# CTIME shift in bits.
timeConversion.setCTimeShift(2)

if (mc):
    dbImporter.setIOV(1002, 0, 1002, -1)
    timeConversion.setCTimeShift(2)
    dbImporter.importTimeConversion(timeConversion)

    dbImporter.setIOV(1003, 0, 1003, -1)
    timeConversion.setCTimeShift(0)
    dbImporter.importTimeConversion(timeConversion)

    dbImporter.setIOV(0, 0, 0, -1)
    timeConversion.setCTimeShift(0)
    dbImporter.importTimeConversion(timeConversion)

else:
    dbImporter.setIOV(0, 0, 3, -1)
    timeConversion.setCTimeShift(2)
    dbImporter.importTimeConversion(timeConversion)

    dbImporter.setIOV(4, 0, -1, -1)
    timeConversion.setCTimeShift(0)
    dbImporter.importTimeConversion(timeConversion)
