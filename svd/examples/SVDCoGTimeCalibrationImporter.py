#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Script to Import Calibrations into a local DB
"""

from basf2 import *
import ROOT
from ROOT.Belle2 import SVDCoGTimeCalibrationsImporter
from ROOT.Belle2 import FileSystem
import os
import sys
import glob
import subprocess
import interactive
import argparse
from fnmatch import fnmatch

parser = argparse.ArgumentParser(description="SVD CoG Time Calibration Importer")
parser.add_argument('--exp', metavar='expNumber', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='runNumber', dest='run', type=int, nargs=1, help='Run Number')

'''
if(len(sys.argv) != 7):
    print('')
    print('ERROR: wrong number of arguments passed, check below the correct usage of this script.')
    print('')
    print('provided input arguments')
    print(sys.argv)
    parser.print_help()
    exit()
'''
print('')

args = parser.parse_args()

experiment = args.exp[0]
run = args.run[0]

print('experiment number = ' + str(experiment))
print('       run number = ' + str(run))
reset_database()
use_local_database("localDB/database.txt", "localDB")

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': experiment, 'runList': run})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
main.add_module("Gearbox")

# the calibrations are good from the NEXT run
run = int(int(run) + 1)


class dbImporterModule(Module):
    def beginRun(self):
        # call the importer class
        #        dbImporter = SVDCoGTimeCalibrationsImporter(experiment, run, experiment, -1)
        dbImporter = SVDCoGTimeCalibrationsImporter(0, 0, -1, -1)
        dbImporter.importNULLCoGTimeCalibrations()


main.add_module(dbImporterModule())

process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
