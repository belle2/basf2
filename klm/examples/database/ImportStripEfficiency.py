#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM strip efficiency payloads.

import sys
import basf2
import ROOT
from ROOT.Belle2 import KLMDatabaseImporter

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

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importStripEfficiency(inputFile)

dbImporter.setIOV(1002, 0, 1002, -1)
dbImporter.importStripEfficiency(inputFile)

dbImporter.setIOV(1003, 0, 1003, -1)
dbImporter.importStripEfficiency(inputFile)
