#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
from ROOT.Belle2 import TrgEclDatabaseImporter
from ROOT.Belle2 import FileSystem
import glob


# register event info setter
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()

# register gearbox
gearbox = register_module('Gearbox')
gearbox.initialize()


FAMParameters = FileSystem.findFile('trg/ecl/data/FAM_Parameters.dat')
SignalPDF = FileSystem.findFile('trg/ecl/data/SignalPDF.dat')
NoiseMatrix = FileSystem.findFile('trg/ecl/data/NoiseMatrix.dat')

TMMParameters = FileSystem.findFile('trg/ecl/data/TMM_Parameters.dat')
ETMParameters = FileSystem.findFile('trg/ecl/data/ETM_Parameters.dat')
BadRunList = FileSystem.findFile('trg/ecl/data/BadRunList.dat')

use_local_database("trg_ecl/database.txt", "trg_ecl")

dbImporter = TrgEclDatabaseImporter()

dbImporter.setExpNumber(0, -1)
dbImporter.setRunNumber(0, -1)

dbImporter.importFAMParameter(FAMParameters, NoiseMatrix, NoiseMatrix)
dbImporter.importTMMParameter(TMMParameters)
dbImporter.importETMParameter(ETMParameters)
dbImporter.importBadRunNumber(BadRunList)
