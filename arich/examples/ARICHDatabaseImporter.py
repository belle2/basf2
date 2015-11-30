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

# use_local_database("test_database.txt", "test_payloads", LogLevel.WARNING)
# reset_database()
use_local_database()
# use_central_database("test_param", LogLevel.WARNING)
conditionsdb = register_module("Conditions")

# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()

# download aerogel data if it is not available locally
if not os.path.isfile('aerogel/AerogelData.xml'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/aerogel_2nd/'])

# create a gearbox module to read read the aerogel data so it can be used
# it needs to open just the aerogel/AerogelData.xml which includes all other xml
# files in that folder. What we do is that we set the backend to point to that
# folder so that gearbox can find all files in there.
paramloader = register_module('Gearbox')
paramloader.param('backends', ['file:AllData/'])
paramloader.param('fileName', 'ArichData.xml')
paramloader.initialize()

# create a std::vector<string>
rootFiles = ROOT.vector('string')()
# and add the files we want to read
if not os.path.isfile('hapdQA_test/KA0152_data.root'):
    subprocess.call(['svn', 'co', 'https://belle2.cc.kek.jp/svn/groups/arich/database/data/hapdQA_test/'])
xdirHapdQA = '/home/manca/belle2/head/arich/testdb/hapdQA_test/'
for path, subdirs, files in os.walk(xdirHapdQA):
    for name in files:
        if fnmatch(name, "*.root"):
            rootFiles.push_back(os.path.join(path, name))

# create a std::vector<string>
rootFiles2 = ROOT.vector('string')()
txtFiles2 = ROOT.vector('string')()
# and add the files we want to read
xdir = '/home/manca/belle2/head/arich/dbdata/asicData/'
for path, subdirs, files in os.walk(xdir):
    for name in files:
        if fnmatch(name, "*.root"):
            rootFiles2.push_back(os.path.join(path, name))
        if fnmatch(name, "*.txt"):
            txtFiles2.push_back(os.path.join(path, name))

main = create_path()
main.add_module(eventinfo)
main.add_module(conditionsdb)
process(main)

# and run the importer
dbImporter = ARICHDatabaseImporter(rootFiles, rootFiles2, txtFiles2)
# dbImporter.importAerogelInfo()
# dbImporter.exportAerogelInfo()
# dbImporter.importHapdQA()
# dbImporter.exportHapdQA()
dbImporter.importAsicInfo()
dbImporter.exportAsicInfo()
# dbImporter.importFebTest()
# dbImporter.exportChannelsList()
