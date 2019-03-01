#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Example of importing common T0 constants determined with cdst_calibrateCommonT0.py
# ---------------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
from ROOT import TH1F, TFile
import os
import sys
import glob


# Create path
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1])
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Initialize TOP geometry parameters from gearbox
main.add_module('TOPGeometryParInitializer', useDB=False)

# process single event
process(main)

# define a local database (will be created automatically, if doesn't exist)
use_local_database("localDB/localDB.txt", "localDB", False)

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants

# root file names are supposed to be 'commonT0_r*.root', where * is a run number
pathToFiles = '.'  # set the correct path to root files!
allFileNames = sorted(glob.glob(pathToFiles + '/commonT0_r*.root'))
fileNames = []
for fileName in allFileNames:
    file = TFile.Open(fileName)
    h = file.Get('commonT0')
    if not h:
        B2ERROR('no histogram with name commonT0')
        file.Close()
        continue
    tree = file.Get('tree')
    if not tree:
        B2ERROR('no TTree with name tree')
        file.Close()
        continue
    if tree.GetEntries() > 10:  # require some minimal number of tracks in a run
        fileNames.append(fileName)
    file.Close()

numFiles = len(fileNames)
if numFiles == 0:
    print('No files found')
    sys.exit()

expNo = 3  # set the correct experiment number
lastRunNo = 5613  # set the correct last run number
bunchTimeSep = 47.163878 / 24
for i, fileName in enumerate(fileNames):
    runFirst = int((fileName.split('commonT0_r')[1]).split('.')[0])
    runLast = lastRunNo
    k = i + 1
    if k < len(fileNames):
        nextName = fileNames[k]
        runLast = int((nextName.split('commonT0_r')[1]).split('.')[0]) - 1
    if runLast < runFirst:
        B2ERROR("first run:", runFirst, "last run:", runLast)
        B2ERROR("Last run is less than the first one: exiting!")
        sys.exit()

    file = TFile.Open(fileName)
    h = file.Get('commonT0')
    t0 = h.GetBinContent(1)
    t0 -= round(t0 / bunchTimeSep, 0) * bunchTimeSep
    err = h.GetBinError(1)
    dbImporter.importCommonT0(t0, err, expNo, runFirst, runLast)
    file.Close()

B2RESULT("Done")
