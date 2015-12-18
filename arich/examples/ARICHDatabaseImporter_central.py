#!/usr/bin/env python
# -*- coding: utf-8 -*-
from basf2 import *
import ROOT
from ROOT.Belle2 import ARICHDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

use_central_database("test_ARICHReconstruction", LogLevel.ERROR)
# use_central_database("test_param", LogLevel.WARNING)

# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()

main = create_path()
main.add_module(eventinfo)
process(main)

# and run the importer
dbImporter = ARICHDatabaseImporter(
    ROOT.vector('string')(),
    ROOT.vector('string')(),
    ROOT.vector('string')(),
    ROOT.vector('string')())

dbImporter.exportAerogelInfo()
# dbImporter.exportHapdQA()
# dbImporter.exportAsicInfo()
# dbImporter.exportFebTest()
# dbImporter.exportHapdInfo()
# dbImporter.exportHapdChipInfo()
dbImporter.exportHapdQE()
dbImporter.exportBadChannels()

# simple example that shows how to read data from databse and use it
# dbImporter.getBiasVoltagesForHapdChip("KA0167")
