#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Get info about TBC constants in database
#
# usage: basf2 getTBCInfo.py expNo runNo [globalTag]
# ---------------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter
import sys

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
    b2.use_central_database(tag)

# create path
main = b2.create_path()

# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'runList': [runNo], 'expList': [expNo]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = b2.register_module('Gearbox')
main.add_module(gearbox)

# Geometry
geometry = b2.register_module('Geometry')
geometry.param('useDB', False)
geometry.param('components', ['TOP'])
main.add_module(geometry)

# process single event
b2.process(main)

print()
print('Experiment =', expNo, 'Run =', runNo, 'global tag =', tag)
print()

# and then run the importer
dbImporter = TOPDatabaseImporter()
dbImporter.getSampleTimeCalibrationInfo()
