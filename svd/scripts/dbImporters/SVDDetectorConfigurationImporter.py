#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
SVD Detector Configuration Importer.
Script to the configuration parameters, Local and Global, into a local DB
"""

import basf2 as b2
from ROOT.Belle2 import SVDDetectorConfigurationImporter
import sys
import argparse
from termcolor import colored
from basf2 import conditions as b2conditions

parser = argparse.ArgumentParser(description="SVD Detector Configuration Importer")
parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='run', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--cfgXML', metavar='config xml', dest='calib', type=str, nargs=1, help='GlobalRun Calibration XML file')

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
print('|            global xml = ' + str(calibfile))
print('| --->   THANKS!   <---')
sys.stdout.write(RESET)

print('')

proceed = input("Do you want to proceed? y/n ")
if not str(proceed) == 'y':
    print(colored(str(proceed) + ' != y, therefore we exit now', 'red'))
    exit(1)

b2conditions.prepend_globaltag("svd_basic")

main = b2.create_path()


# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': experiment, 'runList': run})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
main.add_module("Gearbox")

run = int(run)


class configImporterToDBModule(b2.Module):
    '''detector configuration importer'''

    def beginRun(self):
        '''begin run'''

        # call the importer class
        configImporterToDB = SVDDetectorConfigurationImporter(experiment, run, experiment, -1)
        if args.calib is not None:
            # import SVDGlobalConfigParameters dbobject: ZS, latency, mask, APV clock units
            configImporterToDB.importSVDGlobalConfigParametersFromXML(calibfile)
            print(colored("V) Global Detector Configuration parameters: (ZS, latency, mask, APVClock units) Imported", 'green'))
            # import SVDLocalConfigParameters dbobject: calibration peak and time units, date
            configImporterToDB.importSVDLocalConfigParametersFromXML(calibfile)
            print(colored("V) Local Detector Configuration parameters Imported", 'green'))
        else:
            print(colored("X) Detector Configuration parameters are not NOT imported.", 'red'))


main.add_module(configImporterToDBModule())

b2.process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
