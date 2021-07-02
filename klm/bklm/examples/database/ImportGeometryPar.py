#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import BKLM geometry parameters payloads.

import basf2
from ROOT.Belle2 import BKLMDatabaseImporter

basf2.set_log_level(basf2.LogLevel.INFO)

# The Gearbox is needed for BKLMGeometryPar
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.initialize()
gearbox = basf2.register_module('Gearbox')
gearbox.initialize()

dbImporter = BKLMDatabaseImporter()

dbImporter.setIOV(0, 0, 0, -1)
dbImporter.importGeometryPar()
