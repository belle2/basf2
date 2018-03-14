#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

from basf2 import *
import ROOT
from ROOT.Belle2 import EKLMDatabaseImporter, EKLMChannelData

set_log_level(LogLevel.INFO)

eventinfosetter = register_module('EventInfoSetter')

# Gearbox
gearbox = register_module('Gearbox')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

process(main)

dbImporter = EKLMDatabaseImporter()
dbImporter.loadDefaultChannelData()
channel_data = EKLMChannelData()
channel_data.setActive(False)
dbImporter.setChannelData(1, 1, 1, 1, 40, channel_data)
dbImporter.importChannelData()
