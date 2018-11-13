#!/usr/bin/env python
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Get info about TBC constants in database
#
# usage: basf2 getTBCInfo.py expNo runNo [globalTag]
# ---------------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

argvs = sys.argv
if len(argvs) < 3:
    print("usage: basf2", argvs[0], "expNo runNo [globalTag]")
    sys.exit()
expNo = int(argvs[1])
runNo = int(argvs[2])
tag = '(default)'
if len(argvs) == 4:
    tag = argvs[3]

# Central database
if len(argvs) == 4:
    use_central_database(tag)

# create path
main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [runNo], 'expList': [expNo]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
process(main)

print()
print('Experiment =', expNo, 'Run =', runNo, 'global tag =', tag)
print()

# and then run the importer
dbImporter = TOPDatabaseImporter()
dbImporter.getSampleTimeCalibrationInfo()
