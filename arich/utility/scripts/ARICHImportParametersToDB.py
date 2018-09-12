#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import ROOT
from ROOT.Belle2 import ARICHDatabaseImporter
import os
import sys
import glob
import subprocess
from fnmatch import fnmatch

# set local database folder
use_local_database("localdb/database.txt",
                   "localdb", 0, LogLevel.INFO)


# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = register_module('EventInfoSetter')
eventinfo.initialize()


# load gearbox for reading parameters from xml files (by default in "arich/data")
paramloader = register_module('Gearbox')
paramloader.initialize()

main = create_path()
main.add_module(eventinfo)
process(main)

# and run the importer
dbImporter = ARICHDatabaseImporter()

# uncomment/comment the desired function

# import modules info
# ARICHModulesInfo is a lightweight DB class, used by ARICH simulation/reconstruction software,
# that holds information of installed and active HAPD modules and their 2D QE maps.
# This function goes through arich/data/ARICH-InstalledModules.xml, finds QE maps of corresponding HAPDs in
# the database (payload "dbstore_ARICHHapdQE_rev_*.root" must exist in the DB) and
# creates ARICHModulesInfo which is then stored in DB

# dbImporter.importModulesInfo()


# import channel mask
# ARICHChannelMask is a lightweight DB class, used by ARICH simulation/reconstruction software,
# that holds list of dead channels of all installed HAPD modules.
# This function goes through arich/data/ARICH-InstalledModules.xml, finds list of dead channels of
# corresponding HAPDs in the database (payload "dbstore_ARICHModuleTest_rev_*.root" must exist in the DB) and
# creates ARICHChannelMask which is then stored in DB

# dbImporter.importChannelMask()


# import simulation parameters
# ARICHSimulationPar is DB class, used by ARICH simulation/reconstruction software,
# that holds QE vs. wavelenght curve, and some other parameters of photon detection.
# The parameters are read from arich/data/ARICH-SimulationPar.xml

# dbImporter.importSimulationParams()

# import reconstruction parameters
# at this point it only initializes "default" values, defined in ARICHReconstructionPar class itself

# dbImporter.importReconstructionParams()


# import channel mapping
# ARICHChannelMapping is a DB class, used by ARICH simulation/reconstruction software,
# that holds mapping of HAPD channel asic numbers to channel (x,y) position numbers (0,11)
# The mapping is read from arich/data/ARICH-ChannelMapping.xml

# dbImporter.importChannelMapping()


# import front-end mappings
# ARICHMergerMapping and ARICHCopperMapping are DB classes that hold mapping of HAPD modules
# to merger boards and mapping of mereger boards to copper boards
# Mappings are read from the arich/data/ARICH-FrontEndMapping.xml

# dbImporter.importFEMappings()

# Import geometry parameters from xml files to the database
# Parameters are read from arich/data/ARICH-(Aerogel,Detector,Hapd,Mirrors).xml

# dbImporter.importGeometryConfig()

# Import simple cosmic test geometry (i.e. one aerogel tile + scintilators).
# Parameters are read from arich/data/ARICH-CosmicTest.xml

# dbImporter.importCosmicTestGeometry()

# below few functions print the content of described classes from the database

# dbImporter.printModulesInfo()
# dbImporter.printSimulationPar()
# dbImporter.printChannelMask()
# dbImporter.printChannelMapping()
# dbImporter.printFEMappings()


# creates root file with full detector plane QE map (all HAPDs) as stored in the database

# dbImporter.dumpQEMap()

# creates root file with numbering of HAPD module slots (position on detector plane -> module number)

# dbImporter.dumpModuleNumbering()

# print geometry parameters of ARICH detector from the database

# dbImporter.printGeometryConfig()
