#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Simple example for testing the import to database.
"""

from basf2 import *
import ROOT
from ROOT.Belle2 import SVDDatabaseImporter
from ROOT.Belle2 import FileSystem
import os
import sys
import glob
import subprocess
import interactive
from fnmatch import fnmatch

use_local_database("localDB_run400_toImport/database_run400_toImport.txt", "localDB_run400_toImport")
# use_central_database("GT_gen_prod_004.11_Master-20171213-230000")

# use_local_database("test/database.txt", "test")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': [3], 'runList': [400]})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
gearbox = register_module('Gearbox')
main.add_module(gearbox)


class dbImporterModule(Module):
    def beginRun(self):
        # create the importer class
        dbImporter = SVDDatabaseImporter(0, 0, -1, -1)

        # read the map from xml
        dbImporter.importSVDChannelMapping("Hao_mapping.xml")

        # read the noise from xml
        dbImporter.importSVDNoiseCalibrationsFromXML("Hao_noise.xml")
        # read the waveform calibrations from xml
        dbImporter.importSVDCalAmpCalibrationsFromXML("Hao_noise.xml")


'''
# import the calibration constants from local runs
dbImporter.importSVDPulseShapeCalibrations()

print("importPulseShape_Done")

# import the time shift calibration constants
# for the basic time estimator
dbImporter.importSVDTimeShiftCorrections()

print("importTimeShiftCorrection_Done")

# import the bad strips status
dbImporter.importSVDLocalRunBadStrips()

print("importBadStrips_Done")

dbImporter.importSVDChannelMapping()
'''

main.add_module(dbImporterModule())
# process single event
process(main)

print("IMPORT COMPLETED!!!")
