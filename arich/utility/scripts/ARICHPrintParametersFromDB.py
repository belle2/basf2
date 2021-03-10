#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# this prints parameters from the database for a given tag, experiment and run number
# Author: luka.santelj@ijs.si

import basf2 as b2
from ROOT.Belle2 import ARICHDatabaseImporter
from optparse import OptionParser

parser = OptionParser()
parser.add_option('-t', '--tag', dest='tag', default='',
                  help='database tag from which to import/export data')
parser.add_option('-e', '--exp', dest='experiment',
                  default=-1, help='experiment')
parser.add_option('-r', '--run', dest='run',
                  default=-1, help='run')
parser.add_option('-o', '--object', dest='object',
                  default="", help='object to be printed')


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
    dbImporter.printModulesInfo()
elif options.object == 'ARICHSimulationPar':
    dbImporter.printSimulationPar()
elif options.object == 'ARICHChannelMask':
    dbImporter.printChannelMask()  # use dbImporter.printChannelMask(True) to make 2d map of active channels
elif options.object == 'ARICHChannelMapping':
    dbImporter.printChannelMapping()
elif options.object == 'ARICHMergerMapping':
    dbImporter.printMergerMapping()
elif options.object == 'ARICHCopperMapping':
    dbImporter.printCopperMapping()
elif options.object == 'ARICHReconstructionPar':
    dbImporter.printReconstructionPar()
elif options.object == 'ARICHGlobalAlignment':
    dbImporter.printGlobalAlignment()
elif options.object == 'ARICHMirrorAlignment':
    dbImporter.printMirrorAlignment()
elif options.object == 'ARICHAeroTilesAlignment':
    dbImporter.printAeroTilesAlignment()
elif options.object == 'ARICHGeometryConfig':
    # print geometry parameters of ARICH detector from the database
    dbImporter.printGeometryConfig()
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
    dbImporter.printFEMappings()
else:
    print('Non-valid arich DB object!')
