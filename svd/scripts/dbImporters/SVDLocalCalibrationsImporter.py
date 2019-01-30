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
from termcolor import colored

parser = argparse.ArgumentParser(description="SVD Local Calibrations Importer")
parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='run', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--cfgXML', metavar='config xml', dest='calib', type=str, nargs=1, help='GlobalRun Calibration XML file')
parser.add_argument('--mapXML', metavar='channel map xml', dest='mapp', type=str, nargs=1, help='Channel Mapping xml file')
parser.add_argument('--isLocalXML', dest='localXml', action='store_const', const=True, default=False,
                    help='Add this parameter if the XML is a Local run configuration instead of a Global Run Configuration')
parser.add_argument('--nomask', dest='mask', action='store_const', default=False, const=True,
                    help='Add this parameter if the XML does not contain the attribute <masks>')

print('')

if(str(sys.argv[1]) == "help"):
    parser.print_help()
    exit(1)

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

RED = "\033[1;31m"
BLUE = "\033[1;34m"
CYAN = "\033[1;36m"
GREEN = "\033[0;32m"
RESET = "\033[0;0m"
BOLD = "\033[;1m"
BLEU = "\033[34m"
REVERSE = "\033[;7m"
sys.stdout.write(RED)
print('| ---> CHECK HERE: <---')
print('|     experiment number = ' + str(experiment))
print('|first valid run number = ' + str(run))
print('|       calibration xml = ' + str(calibfile))
print('|           mapping xml = ' + str(mappingfile))
print('|   is a global run xml = ' + str(not args.localXml))
print('|          import masks = ' + str(not args.mask))
print('| --->   THANKS!   <---')
sys.stdout.write(RESET)

print('')

proceed = input("Do you want to proceed? y/n ")
if not str(proceed) == 'y':
    print(colored(str(proceed) + ' != y, therefore we exit now', 'red'))
    exit(1)

reset_database()
use_database_chain()
# central DB needed for the channel mapping DB object
GLOBAL_TAG = "vxd_commissioning_20181030"
# GLOBAL_TAG="svd_Belle2_20181221"
use_central_database(GLOBAL_TAG)
use_local_database("localDB/database.txt", "localDB", invertLogging=True)

# local tag and database needed for commissioning

main = create_path()


# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': experiment, 'runList': run})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
main.add_module("Gearbox")

run = int(run)

# TO DO, enable calibration of calib or mapping, depending on the user input


class dbImporterModule(Module):
    def beginRun(self):
        # call the importer class
        dbImporter = SVDLocalCalibrationsImporter(experiment, run, experiment, -1)
        if args.calib is not None:
            # import the noises
            dbImporter.importSVDNoiseCalibrationsFromXML(calibfile)
            print(colored("V) Noise Imported", 'green'))
            # import the pedestals
            dbImporter.importSVDPedestalCalibrationsFromXML(calibfile)
            print(colored("V) Pedestal Imported", 'green'))
            # import pulse shape calibrations
            dbImporter.importSVDCalAmpCalibrationsFromXML(calibfile)
            print(colored("V) Pulse Shape Calibrations Imported", 'green'))
            if not args.localXml:
                # import channel mapping
                dbImporter.importSVDGlobalXMLFile(calibfile)
                print(colored("V) Global Run Configuration xml payload file Imported", 'green'))
            else:
                print(colored("X) Global Run Configuration xml payload file is NOT imported.", 'red'))
            # import FADCMasked strips only if NOT --nomask
            if not args.mask:
                dbImporter.importSVDFADCMaskedStripsFromXML(calibfile)
                print(colored("V) FADC Masked Strips Imported", 'green'))
            else:
                print(colored("(X) FADC Masked Strips are NOT imported.", 'red'))

        if args.mapp is not None:
            # import channel mapping
            dbImporter.importSVDChannelMapping(mappingfile)
            print(colored("V) Channel Mapping Imported", 'green'))


main.add_module(dbImporterModule())

process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
