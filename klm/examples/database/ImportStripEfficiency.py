#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Import KLM strip efficiency payloads.

import sys
from ROOT.Belle2 import KLMDatabaseImporter, KLMStripEfficiency, KLMChannelIndex

mc = False
if (len(sys.argv) < 2):
    print('Usage: basf2 ImportStripEfficiency.py [mc | input_file].')
elif (sys.argv[1] == 'mc'):
    mc = True
else:
    inputFile = sys.argv[1]

dbImporter = KLMDatabaseImporter()

stripEfficiency = KLMStripEfficiency()

if (mc):
    index = KLMChannelIndex()
    index2 = KLMChannelIndex()
    while (index != index2.end()):
        channel = index.getKLMChannelNumber()
        stripEfficiency.setEfficiency(channel, 1.0, 0.0)
        index.increment()

    dbImporter.setIOV(0, 0, 0, -1)
    dbImporter.importStripEfficiency(stripEfficiency)

    dbImporter.setIOV(1002, 0, 1002, -1)
    dbImporter.importStripEfficiency(stripEfficiency)

    dbImporter.setIOV(1003, 0, 1003, -1)
    dbImporter.importStripEfficiency(stripEfficiency)

else:
    dbImporter.loadStripEfficiency(stripEfficiency, inputFile)

    dbImporter.setIOV(0, 0, -1, -1)
    dbImporter.importStripEfficiency(stripEfficiency)
