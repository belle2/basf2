#!/usr/bin/env python
# -*- coding: utf-8 -*-

# --------------------------------------------------------------------------------
# Import dummy payloads
# --------------------------------------------------------------------------------

from basf2 import *
import ROOT
from ROOT.Belle2 import TOPDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch


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
use_local_database("localDB/localDB.txt")

# and then run the importer
dbImporter = TOPDatabaseImporter()

# import constants
dbImporter.importDummyCalModuleAlignment()
dbImporter.importDummyCalModuleT0()
dbImporter.importDummyCalChannelT0()
dbImporter.importDummyCalTimebase()
dbImporter.importDummyCalChannelNoise()
dbImporter.importDummyCalChannelPulseHeight()
dbImporter.importDummyCalChannelRQE()
dbImporter.importDummyCalChannelThresholdEff()
dbImporter.importDummyCalChannelThreshold()
dbImporter.importDummyCalCommonT0()
dbImporter.importDummyCalIntegratedCharge()
dbImporter.importDummyCalAsicShift()
