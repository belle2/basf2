#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Importing module T0 constants saved in histograms to local DB.
# Input: root files from cdst_calibrateModuleT0.py or cdst_chi2ModuleT0calibration.py
#
# usage: basf2 importModuleT0.py pathToRootFiles localDB
#
# ---------------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
from ROOT import TH1F, TFile
import os
import sys
import glob

# Argument parsing
argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "pathToRootFiles localDB")
    sys.exit()
pathToFiles = argvs[1]
localDB = argvs[2]

# Determine experiment number from the path name
try:
    expNo = int((pathToFiles.split('/e')[-1]).split('/')[0])
except:
    B2ERROR("Cannot determine experiment number from path name: " + pathToFiles)
    sys.exit()

# Check root files and prepare IOV's
allFileNames = sorted(glob.glob(pathToFiles + '/moduleT0*.root'))
fileNames = []
runFirst = [0, ]
runLast = []
for fileName in allFileNames:
    file = TFile.Open(fileName)
    h = file.Get('moduleT0')
    if not h:
        B2WARNING(fileName + ': no histogram with name moduleT0, file skipped.')
        file.Close()
        continue
    file.Close()
    try:
        runNum = int(fileName.split('_to_r')[-1].split('.root')[0])
        runFirst.append(runNum + 1)
        runLast.append(runNum)
        fileNames.append(fileName)
    except:
        B2WARNING(fileName +
                  ': cannot determine last run number from file name, file skipped.')
        continue

if len(fileNames) == 0:
    B2ERROR('No selected files found in ' + pathToFiles)
    sys.exit()

runFirst.pop()
runLast[-1] = -1

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
use_local_database(localDB, readonly=False)

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants
print()
print('---- Importing constants for experiment ' + str(expNo) + ' ---->')
print()
for i, fileName in enumerate(fileNames):
    dbImporter.importModuleT0(fileName, expNo, runFirst[i], runLast[i])
    print()

B2RESULT("Done. Constants imported to " + localDB)
