#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Database importer.
Script to Import SVD Channel Mapping into a local DB
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
parser.add_argument('--mapXML', metavar='channel map xml', dest='mapp', type=str, nargs=1, help='Channel Mapping xml file')

print('')

if(str(sys.argv[1]) == "help"):
    parser.print_help()
    exit(1)

args = parser.parse_args()

experiment = args.exp[0]
run = args.run[0]

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
print('|           mapping xml = ' + str(mappingfile))
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


class dbImporterModule(b2.Module):
    '''channel mapping importer module'''

    def beginRun(self):
        '''begin run'''

        # call the importer class
        dbImporter = SVDLocalCalibrationsImporter(experiment, run, experiment, -1)
        if args.mapp is not None:
            # import channel mapping
            dbImporter.importSVDChannelMapping(mappingfile)
            print(colored("V) Channel Mapping Imported", 'green'))


main.add_module(dbImporterModule())

b2.process(main)

print("IMPORT COMPLETED, check the localDB folder and then proceeed with the upload to the central DB")
