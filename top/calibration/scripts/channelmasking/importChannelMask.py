#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
from ROOT import gSystem
import os
import sys
import glob

# Create path
main = create_path()

# Event info setter - execute single event
main.add_module('EventInfoSetter', evtNumList=[1])

# Initialize TOP geometry parameters using default GT
main.add_module('TOPGeometryParInitializer')

# process single event
process(main)

# define a local database (will be created automatically, if doesn't exist)
use_local_database('localDB/localDB.txt', readonly=False)

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants
fileNames = sorted(glob.glob('masks/*.root'))
numFiles = len(fileNames)
if numFiles is 0:
    print('No files found')
    sys.exit()

if not os.path.exists('masks/imported'):
    os.makedirs('masks/imported')

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
