#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Script to Import Calibrations into a local DB
"""

from basf2 import *
import ROOT
from ROOT.Belle2 import SVDLocalCalibrationsImporter
from ROOT.Belle2 import FileSystem
import os
import sys
import glob
import subprocess
import interactive
import argparse
from fnmatch import fnmatch

parser = argparse.ArgumentParser(description="SVD Local Calibrations Importer")
parser.add_argument('--exp', metavar='expNumber', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='runNumber', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--cal_xml', metavar='calibFile', dest='calib', type=str, nargs=1, help='Calibration xml file')
parser.add_argument('--map_xml', metavar='mapFile', dest='mapp', type=str, nargs=1, help='Channel Mapping xml file')
parser.add_argument('--nomask', metavar='maskField', dest='mask', type=int, nargs=1,
                    help='Old xml format with no mask field corresponds to nomask = 1')

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

if args.mask is not None:
    masking = args.mask[0]
else:
    masking = args.mask


print('experiment number = ' + str(experiment))
print('       run number = ' + str(run))
print('  calibration xml = ' + str(calibfile))
print('      mapping xml = ' + str(mappingfile))
print('      no_masks = ' + str(masking))

reset_database()
use_database_chain()
# central DB needed for the channel mapping DB object
use_central_database("Calibration_Offline_Development")
use_local_database("localDB/database.txt", "localDB")

# global tag and database needed for commissioning
# GLOBAL_TAG = "vxd_commissioning_20181030"
# use_central_database(GLOBAL_TAG)

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
        dbImporter = SVDLocalCalibrationsImporter(experiment, run, experiment, -1)
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
            # import FADCMasked strips
            if args.mask is None:
                dbImporter.importSVDFADCMaskedStripsFromXML(calibfile)
                print("FADC Masked Strips Imported")
            elif (masking == 1):
                print("FADC Masked Strips can not be imported. The local calibration xml file has NO masks field!")
        if args.mapp is not None:
            # import channel mapping
            dbImporter.importSVDChannelMapping(mappingfile)
            print("Channel Mapping Imported")


main.add_module(dbImporterModule())

process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
