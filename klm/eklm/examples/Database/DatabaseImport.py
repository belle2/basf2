#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Create EKLM database payloads.

import basf2
from ROOT.Belle2 import EKLMDatabaseImporter

basf2.set_log_level(basf2.LogLevel.INFO)

eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.initialize()
gearbox = basf2.register_module('Gearbox')
gearbox.initialize()

dbImporter = EKLMDatabaseImporter()
# dbImporter.importReconstructionParameters()
dbImporter.importSimulationParameters()
