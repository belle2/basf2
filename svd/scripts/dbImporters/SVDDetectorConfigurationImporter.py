#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Detector Configuration Importer.
Script to the configuration parameters, Local and Global, into a local DB
"""

from basf2 import *
import ROOT
from ROOT.Belle2 import SVDDetectorConfigurationImporter
from ROOT.Belle2 import FileSystem
import os
import sys
import glob
import subprocess
import interactive
import argparse
from fnmatch import fnmatch
from termcolor import colored

parser = argparse.ArgumentParser(description="SVD Detector Configuration Importer")
parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='run', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--cfgXML', metavar='config xml', dest='calib', type=str, nargs=1, help='GlobalRun Calibration XML file')
# parser.add_argument('--isLocalXML', dest='localXml', action='store_const', const=True, default=False,
#                    help='Add this parameter if the XML is a Local run configuration instead of a Global Run Configuration')
# parser.add_argument('--nomask', dest='mask', action='store_const', default=False, const=True,
#                    help='Add this parameter if the XML does not contain the attribute <masks>')

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
# GLOBAL_TAG = "vxd_commissioning_20181030"
GLOBAL_TAG = "svd_Belle2_20181221"
use_central_database(GLOBAL_TAG)
use_local_database("localDB_test/database.txt", "localDB_test", invertLogging=True)

# local tag and database needed for commissioning

main = create_path()


# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': experiment, 'runList': run})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
main.add_module("Gearbox")

run = int(run)


class configImporterToDBModule(Module):
    def beginRun(self):
        # call the importer class
        configImporterToDB = SVDDetectorConfigurationImporter(experiment, run, experiment, -1)
        if args.calib is not None:
            # import SVDGlobalConfigParameters dbobject if NOT --isLocalXML: ZS, latency, mask, APV clock units
            configImporterToDB.importSVDGlobalConfigParametersFromXML(calibfile)
            print(colored("V) Global Detector Configuration parameters", green))
            print(colored("(ZS, latency, mask, APV clock time units) Imported", 'green'))
            # import SVDLocalConfigParameters dbobject if NOT --isLocalXML:
            # injected charge, calibration time units, calibration date
            # configImporterToDB.importSVDLocalConfigParametersFromXML(calibfile)
            # print(colored("V) Local Detector Configuration parameters
            # (injected charge, calibration time units, calibration date) Imported", 'green'))
        else:
            print(colored("X) Detector Configuration parameters are not NOT imported.", 'red'))


main.add_module(configImporterToDBModule())

process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
