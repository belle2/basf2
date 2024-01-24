#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM channel status payloads.

import sys
import basf2
from ROOT.Belle2 import KLMDatabaseImporter, KLMChannelStatus

basf2.set_log_level(basf2.LogLevel.INFO)

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
