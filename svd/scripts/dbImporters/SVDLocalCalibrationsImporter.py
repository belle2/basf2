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
SVD Database importer.
Script to Import Calibrations into a local DB
"""

import basf2 as b2
from ROOT.Belle2 import SVDLocalCalibrationsImporter
import sys
import argparse
from termcolor import colored
from basf2 import conditions as b2conditions

parser = argparse.ArgumentParser(description="SVD Local Calibrations Importer")
parser.add_argument('--exp', metavar='experiment', dest='exp', type=int, nargs=1, help='Experiment Number, = 1 for GCR')
parser.add_argument('--run', metavar='run', dest='run', type=int, nargs=1, help='Run Number')
parser.add_argument('--cfgXML', metavar='config xml', dest='calib', type=str, nargs=1, help='GlobalRun Calibration XML file')
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
print('|   is a global run xml = ' + str(not args.localXml))
print('|          import masks = ' + str(not args.mask))
print('| --->   THANKS!   <---')
sys.stdout.write(RESET)

print('')

proceed = input("Do you want to proceed? y/n ")
if not str(proceed) == 'y':
    print(colored(str(proceed) + ' != y, therefore we exit now', 'red'))
    exit(1)

b2conditions.prepend_globaltag("svd_basic")

# local tag and database needed for commissioning

main = b2.create_path()


# Event info setter - execute single event
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': experiment, 'runList': run})
main.add_module(eventinfosetter)

# Gearbox - access to xml files
main.add_module("Gearbox")

run = int(run)


class dbImporterModule(b2.Module):
    """
    Module to call the importer methods for the payloads creation from XML file
    :param calibfile: path to the xml file containing the local calibrations
    :type calibfile: string
    """

    def beginRun(self):
        """
        Function to call the dbImporter methods to upload the different local payloads
        """
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
        # import FADCMasked strips only if NOT --nomask
            if not args.mask:
                dbImporter.importSVDFADCMaskedStripsFromXML(calibfile)
                print(colored("V) FADC Masked Strips Imported", 'green'))
            else:
                print(colored("X) FADC Masked Strips are NOT imported.", 'red'))
                if not args.localXml:
                    # import XML file only if NOT --isLocalXML
                    dbImporter.importSVDGlobalXMLFile(calibfile)
                    print(colored("V) Global Run Configuration xml payload file Imported", 'green'))
                else:
                    print(colored("X) Global Run Configuration xml payload file is NOT imported.", 'red'))


main.add_module(dbImporterModule())

b2.process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
