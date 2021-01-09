#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import KLM time window payloads.

import basf2
from ROOT.Belle2 import KLMDatabaseImporter, KLMTimeWindow

basf2.set_log_level(basf2.LogLevel.INFO)

dbImporter = KLMDatabaseImporter()

timeWindow = KLMTimeWindow()
timeWindow.setCoincidenceWindow(50)
timeWindow.setPromptTime(0)
# Raw KLM scintillator hit times are in the range from -5000 to -4000 ns approximately.
# The prompt window can be readjusted after the implementation of KLM time calibration.
# timeWindow.setPromptWindow(50)
timeWindow.setPromptWindow(10000)

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importTimeWindow(timeWindow)

dbImporter.setIOV(1002, 0, 1002, -1)
dbImporter.importTimeWindow(timeWindow)

dbImporter.setIOV(1003, 0, 1003, -1)
dbImporter.importTimeWindow(timeWindow)
