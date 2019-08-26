#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2 as b2
import ROOT
from ROOT.Belle2 import BKLMDatabaseImporter

b2.set_log_level(b2.LogLevel.INFO)

# Define the local database
b2.reset_database()
b2.use_local_database("localdb/database.txt", "localdb")

# Simple path with EventInfoSetter and Gearbox
main = b2.create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
b2.process(main)

# And run the importer
dbImporter = BKLMDatabaseImporter()
# dbImporter.importBklmGeometryPar()
dbImporter.importBklmSimulationPar(0, 0, 0, -1)
# dbImporter.importBklmBadChannels()
# dbImporter.importBklmAlignment()
# dbImporter.importBklmMisAlignment()
# dbImporter.importBklmDisplacement()
# dbImporter.importBklmDigitizationParams()
# dbImporter.importBklmADCThreshold()
# dbImporter.importBklmTimeWindow()
