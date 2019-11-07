#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import BKLM time window payloads.

import basf2
import ROOT
from ROOT.Belle2 import BKLMDatabaseImporter, BKLMTimeWindow

basf2.set_log_level(basf2.LogLevel.INFO)

dbImporter = BKLMDatabaseImporter()

time = BKLMTimeWindow()
time.setCoincidenceWindow(50)
time.setPromptTime(0)
time.setPromptWindow(2000)

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importTimeWindow(time)
