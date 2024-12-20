#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# this prints parameters from the database for a given tag, experiment and run number

import basf2 as b2
from ROOT import Belle2  # noqa: make Belle2 namespace available
from ROOT.Belle2 import ARICHDatabaseImporter
from optparse import OptionParser

# basf2 ARICHPrintParametersFromDB.py -- -t data_prompt_rel08 -e 31 -r 13 -o ARICHGlobalAlignment
parser = OptionParser()
parser.add_option('-t', '--tag', dest='tag', default='',
                  help='database tag from which to import/export data')
parser.add_option('-e', '--exp', dest='experiment',
                  default=-1, help='experiment')
parser.add_option('-r', '--run', dest='run',
                  default=-1, help='run')
parser.add_option('-o', '--object', dest='object',
                  default="", help='object to be printed')
parser.add_option('-d', '--dump', dest='dump', action='store_true', default=False, help='dump payload to localdb')


(options, args) = parser.parse_args()

if options.object == '':
    print("use option -o to specify the object to be printed (ARICHReconstructionPar, etc)")

# set database tag
if options.tag == 'local':
    b2.conditions.testing_payloads = ["localdb/database.txt"]
elif options.tag == '':
    print("Using default tag")
else:
    b2.conditions.override_globaltags([options.tag])

# EventInfoSetter is only needed to register EventMetaData...
# (will try to get rid of this)
eventinfo = b2.register_module('EventInfoSetter')
eventinfo.initialize()
main = b2.create_path()

main.add_module(eventinfo)

# load gearbox for reading parameters from xml files (by default in "arich/data")
paramloader = b2.register_module('Gearbox')
paramloader.initialize()

b2.process(main)

# run the importer
dbImporter = ARICHDatabaseImporter()
exp = int(options.experiment)
run = int(options.run)
if exp > 0 and run > 0:
    dbImporter.setExperimentAndRun(exp, run)

if options.object == 'ARICHModulesInfo':
    dbImporter.printModulesInfo(dump=options.dump)
elif options.object == 'ARICHSimulationPar':
    dbImporter.printSimulationPar(dump=options.dump)
elif options.object == 'ARICHChannelMask':
    dbImporter.printChannelMask(dump=options.dump)
    # use dbImporter.printChannelMask(True, dump=options.dump)) to make 2d map of active channels
elif options.object == 'ARICHChannelMapping':
    dbImporter.printChannelMapping(dump=options.dump)
elif options.object == 'ARICHMergerMapping':
    dbImporter.printMergerMapping(dump=options.dump)
elif options.object == 'ARICHCopperMapping':
    dbImporter.printCopperMapping(dump=options.dump)
elif options.object == 'ARICHReconstructionPar':
    dbImporter.printReconstructionPar(dump=options.dump)
elif options.object == 'ARICHGlobalAlignment':
    dbImporter.printGlobalAlignment(dump=options.dump)
elif options.object == 'ARICHMirrorAlignment':
    dbImporter.printMirrorAlignment(dump=options.dump)
elif options.object == 'ARICHAeroTilesAlignment':
    dbImporter.printAeroTilesAlignment(dump=options.dump)
elif options.object == 'ARICHGeometryConfig':
    # print geometry parameters of ARICH detector from the database
    dbImporter.printGeometryConfig(dump=options.dump)
elif options.object == 'moduleNumbering':
    # creates file with numbering of HAPD module slots (position on detector plane -> module number)
    dbImporter.dumpModuleNumbering()
elif options.object == 'QEMap':
    # creates root file with full detector plane QE map (all HAPDs) as stored in the database
    dbImporter.dumpQEMap()
elif options.object == 'MergerMap':
    # creates root file with numbering of HAPD modules to mergers (position on detector plane -> merger SN)
    # use dbImporter.dumpMergerMapping(False) to print merger ID instead SN (ID = (sector-1)*12 + merger_sector_id (1-12))
    dbImporter.dumpMergerMapping()
elif options.object == 'FEMappings':
    # prints FE mappings (module to merger to copper) in json suitable format
    dbImporter.printFEMappings(dump=options.dump)
else:
    print('Non-valid arich DB object!')
