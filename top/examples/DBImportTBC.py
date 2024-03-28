#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------------------------------------------------
# Example of using Database importer for importing TBC constants to local database
# User has to set the path to TOPTimeBaseCalibrator output files correctly
# Name and location of local DB can also be changed
# --------------------------------------------------------------------------------

import basf2 as b2
from ROOT.Belle2 import TOPDatabaseImporter
import sys
import glob

# define local database with write access
b2.conditions.expert_settings(save_payloads="localDB/localDB.txt")

# get a list of root files containing calibration constants
folder_name = 'tbc'  # location of output files from TOPTimeBaseCalibrator
fNames = glob.glob(folder_name + '/*.root')
if len(fNames) == 0:
    print('No root files found in', folder_name)
    sys.exit()

# convert the list to a single string
fileNames = ''
for fName in fNames:
    fileNames = fileNames + ' ' + fName


class PayloadImporter(b2.Module):
    ''' Payload importer using TOPDatabaseImporter '''

    def initialize(self):
        ''' Import timebase calibration '''

        dbImporter = TOPDatabaseImporter()
        dbImporter.importSampleTimeCalibration(fileNames)


# create path
main = b2.create_path()

# Event info setter - execute single event
main.add_module('EventInfoSetter')

# Gearbox
main.add_module('Gearbox')

# Geometry parameters
main.add_module('TOPGeometryParInitializer', useDB=False)

# Importer
main.add_module(PayloadImporter())

# process single event
b2.process(main)
