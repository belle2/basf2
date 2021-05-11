#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT.Belle2 import TrgEclDatabaseImporter
from ROOT.Belle2 import FileSystem


# register event info setter
eventinfo = b2.register_module('EventInfoSetter')
eventinfo.initialize()

# register gearbox
gearbox = b2.register_module('Gearbox')
gearbox.initialize()


FAMParameters = FileSystem.findFile('../data/FAM_Parameters.dat')
SignalPDF = FileSystem.findFile('trg/ecl/data/SignalPDF.dat')
NoiseMatrix = FileSystem.findFile('trg/ecl/data/NoiseMatrix.dat')

TMMParameters = FileSystem.findFile('trg/ecl/data/TMM_Parameters.dat')
ETMParameters = FileSystem.findFile('trg/ecl/data/ETM_Parameters.dat')
BadRunList = FileSystem.findFile('trg/ecl/data/BadRunList.dat')

b2.use_local_database("trg_ecl/database.txt", "trg_ecl")

dbImporter = TrgEclDatabaseImporter()

dbImporter.setExpNumber(0, -1)
dbImporter.setRunNumber(0, -1)

dbImporter.importFAMParameter(FAMParameters, NoiseMatrix, NoiseMatrix)
dbImporter.importTMMParameter(TMMParameters)
dbImporter.importETMParameter(ETMParameters)
dbImporter.importBadRunNumber(BadRunList)
