#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
from ROOT import gSystem
import os
import sys
import glob

# reset_database()
# use_database_chain()
# use_central_database("online")
# use_central_database('data_reprocessing_proc8')
# use_central_database('data_reprocessing_prompt')
# use_central_database('data_reprocessing_prompt_bucket2c',LogLevel.WARNING)
# use_central_database('data_reprocessing_prompt_bucket4_alignment',LogLevel.WARNING)
use_central_database('data_reprocessing_prompt_bucket6_cdst')
# gSystem.Load("libtop.so")

# define a local database (will be created automatically, if doesn't exist)
use_local_database("localDB/localDB.txt", "localDB", False)


main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [1312]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# geometry = register_module('Geometry')
# geometry.param('components', ['TOP'])
# main.add_module(geometry)
# main.add_module(geometry, components=['TOP'])
# main.add_module('TOPGeometryParInitializer')
main.add_module("Geometry")

# process single event
process(main)

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants
fileNames = sorted(glob.glob('masks/*.root'))
numFiles = len(fileNames)
if numFiles is 0:
    print('No files found')
    sys.exit()


for i, fileName in enumerate(fileNames):
    runFirst = int((fileName.split('_r')[1]).split('.')[0])
    expNo = int((fileName.split('_e')[1]).split('_')[0])
    runLast = runFirst
    k = i + 1
    if k < len(fileNames):
        nextName = fileNames[k]
        runLast = int((nextName.split('_r')[1]).split('.')[0]) - 1
        expNext = int((nextName.split('_e')[1]).split('_')[0])
    if expNext > expNo or k == len(fileNames):  # last run in exp or last run in total
        runLast = -1
    elif runLast < runFirst:
        B2ERROR("first run:", runFirst, "last run:", runLast)
        B2ERROR("Last run is less than the first one: exiting!")
        sys.exit()
    dbImporter.importChannelMask(fileName, expNo, runFirst, runLast)
    os.rename(fileName, fileName.replace('masks/', 'masks/imported/'))
B2RESULT("Done")
