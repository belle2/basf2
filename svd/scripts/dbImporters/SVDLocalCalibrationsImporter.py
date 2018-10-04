#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Script to Import Calibrations into a local DB
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
import argparse
from fnmatch import fnmatch

parser = argparse.ArgumentParser(description="SVD Database Importer")
parser.add_argument('--exp', metavar='expNumber', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='runNumber', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--cal_xml', metavar='calibFile', dest='calib', type=str, nargs=1, help='Calibration xml file')
parser.add_argument('--hot_xml', metavar='hotStrFile', dest='hot', type=str, nargs=1, help='Hot Strips xml file')
parser.add_argument(
    '--FADCMasked_xml',
    metavar='FADCMaskedStrFile',
    dest='FADCMasked',
    type=str,
    nargs=1,
    help='FADC Masked Strips xml file')
parser.add_argument('--map_xml', metavar='mapFile', dest='mapp', type=str, nargs=1, help='Channel Mapping xml file')

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
if args.calib is not None:
    calibfile = args.calib[0]
else:
    calibfile = args.calib
if args.mapp is not None:
    mappingfile = args.mapp[0]
else:
    mappingfile = args.mapp
if args.hot is not None:
    hotfile = args.hot[0]
else:
    hotfile = args.hot

if args.FADCMasked is not None:
    FADCMaskedfile = args.FADCMasked[0]
else:
    FADCMaskedfile = args.FADCMasked

print('experiment number = ' + str(experiment))
print('       run number = ' + str(run))
print('  calibration xml = ' + str(calibfile))
print('   hot strips xml = ' + str(hotfile))
print('   FADC Masked strips xml = ' + str(FADCMaskedfile))
print('      mapping xml = ' + str(mappingfile))

reset_database()
use_database_chain()
# central DB needed for the channel mapping DB object
use_central_database("332_COPY-OF_GT_gen_prod_004.11_Master-20171213-230000")
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

# TO DO, enable calibration of calib or mapping, depending on the user input


class dbImporterModule(Module):
    def beginRun(self):
        # call the importer class
        dbImporter = SVDDatabaseImporter(experiment, run, experiment, -1)
        if args.calib is not None:
            # import the noises
            dbImporter.importSVDNoiseCalibrationsFromXML(calibfile)
            print("Noise Imported")
            # import the pedestals
            dbImporter.importSVDPedestalCalibrationsFromXML(calibfile)
            print("Pedestal Imported")
            # import pulse shape calibrations
            dbImporter.importSVDCalAmpCalibrationsFromXML(calibfile)
            print("Pulse Shape Calibrations Imported")
        if args.mapp is not None:
            # import channel mapping
            dbImporter.importSVDChannelMapping(mappingfile)
            print("Channel Mapping Imported")
        if args.hot is not None:
            # import hot strips
            dbImporter.importSVDHotStripsCalibrationsFromXML(hotfile)
            print("Hot Strips List Imported")
            # dbImporter.importSVDHotStripsCalibrations()
        if args.FADCMasked is not None:
            # import FADCMasked strips
            dbImporter.importSVDFADCMaskedStripsFromXML(FADCMaskedfile)
            # print("FADC Masked Strips List Imported")
            # dbImporter.importSVDFADCMaskedStrips()


main.add_module(dbImporterModule())

process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
