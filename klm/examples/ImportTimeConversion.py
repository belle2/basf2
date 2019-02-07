#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM time conversion payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import KLMDatabaseImporter, KLMTimeConversion

basf2.set_log_level(basf2.LogLevel.INFO)

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

eventinfosetter = basf2.register_module('EventInfoSetter')

# Gearbox
gearbox = basf2.register_module('Gearbox')

# Create main path
main = basf2.create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

basf2.process(main)

dbImporter = KLMDatabaseImporter()

timeConversion = KLMTimeConversion()
# TDC frequency in GHz
timeConversion.setTDCFrequency(1.017728000)
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
