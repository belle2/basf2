#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM strip efficiency payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import KLMDatabaseImporter, KLMStripEfficiency, KLMChannelIndex

mc = False
if (len(sys.argv) < 2):
    print('Usage: basf2 ImportStripEfficiency.py [mc | input_file].')
elif (sys.argv[1] == 'mc'):
    mc = True
else:
    inputFile = sys.argv[1]

# Create main path
main = basf2.create_path()
basf2.set_log_level(basf2.LogLevel.INFO)

# EventInfoSetter
main.add_module('EventInfoSetter')

# Gearbox
main.add_module('Gearbox')

# Process the main path
basf2.process(main)

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
