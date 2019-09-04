#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM channel status payloads.

import sys
from ROOT.Belle2 import KLMDatabaseImporter, KLMChannelStatus

mc = False
if (len(sys.argv) >= 2):
    if (sys.argv[1] == 'mc'):
        mc = True

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
