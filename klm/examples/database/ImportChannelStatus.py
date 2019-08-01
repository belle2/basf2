#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM channel status.

import sys
import basf2
import ROOT
import math
from ROOT.Belle2 import KLMDatabaseImporter, KLMChannelStatus

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

channelStatus = KLMChannelStatus()
channelStatus.setStatusAllChannels(KLMChannelStatus.c_Normal)

if (mc):
    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importChannelStatus(channelStatus)

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importChannelStatus(channelStatus)

    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importChannelStatus(channelStatus)

else:
    dbImporter.setIOV(0, 0, -1, -1)
    dbImporter.importChannelStatus(channelStatus)
