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

# use_local_database()
# use_local_database("test_database.txt", "test_payloads")
# use use_central_database for uploading data to PNNL
# use_central_database("ARICHdata", LogLevel.ERROR);
use_central_database("test_ARICHReconstruction", LogLevel.ERROR)


# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()

# download data if it is not available locally
# make sure you change paths
if not os.path.isfile('aerogel_2nd/AerogelData.xml'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/aerogel_2nd/'])
if not os.path.isfile('hapd_hamamatsuData/hapdData.xml'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/hapd_hamamatsuData/'])
if not os.path.isfile('AllData/ArichData.xml'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/AllData/'])
if not os.path.isfile('asicData/ASIC_status.xml'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/asicData/'])
if not os.path.isfile('hapdQA_test/KA0152_data.root'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/hapdQA_test/'])
if not os.path.isfile('febTest/SN_dna.xml'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/febTest/'])

# create a gearbox module to read read the aerogel data so it can be used
# it needs to open just the aerogel/AerogelData.xml which includes all other xml
# files in that folder. What we do is that we set the backend to point to that
# folder so that gearbox can find all files in there.
paramloader = register_module('Gearbox')
pathname = 'file://%s/AllData/' % (os.getcwd())
paramloader.param('backends', [pathname])
paramloader.param('fileName', 'ArichData.xml')
paramloader.initialize()


# create a std::vector<string>
rootFilesHapdQA = ROOT.vector('string')()
# and add the files we want to read
xdirHapdQA = '%s/hapdQA_test/' % (os.getcwd())
for path, subdirs, files in os.walk(xdirHapdQA):
    for name in files:
        if fnmatch(name, "*.root"):
            rootFilesHapdQA.push_back(os.path.join(path, name))

# create a std::vector<string>
rootFilesHapdQE = ROOT.vector('string')()
# and add the files we want to read
# QE can be downloaded from http://research.kek.jp/group/arich/internal/pixel_map/
xdirHapdQE = '%s/hapdQE/' % (os.getcwd())
for path, subdirs, files in os.walk(xdirHapdQE):
    for name in files:
        if fnmatch(name, "*.root"):
            rootFilesHapdQE.push_back(os.path.join(path, name))

# create a std::vector<string>
rootFilesAsics = ROOT.vector('string')()
txtFilesAsics = ROOT.vector('string')()
# and add the files we want to read
xdir = '%s/asicData/' % (os.getcwd())
for path, subdirs, files in os.walk(xdir):
    for name in files:
        if fnmatch(name, "*.root"):
            rootFilesAsics.push_back(os.path.join(path, name))
        if fnmatch(name, "*.txt"):
            txtFilesAsics.push_back(os.path.join(path, name))

main = create_path()
main.add_module(eventinfo)
process(main)

# and run the importer
dbImporter = ARICHDatabaseImporter(rootFilesHapdQA, rootFilesAsics, txtFilesAsics, rootFilesHapdQE)
dbImporter.importAerogelInfo()
# dbImporter.exportAerogelInfo()
# dbImporter.importHapdQA()
# dbImporter.exportHapdQA()
# dbImporter.importAsicInfo()
# dbImporter.exportAsicInfo()
# dbImporter.importFebTest()
# dbImporter.exportFebTest()
# dbImporter.importHapdInfo()
# dbImporter.exportHapdInfo()
# dbImporter.importHapdChipInfo()
# dbImporter.exportHapdChipInfo()
dbImporter.importHapdQE()
# dbImporter.exportHapdQE()
dbImporter.importBadChannels()
# dbImporter.exportBadChannels()

# simple example that shows how to read data from databse and use it
# dbImporter.getBiasVoltagesForHapdChip("KA0167")
