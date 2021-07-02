#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# use to import parmeters from xml files to corresponding database classes
# uncomment the desired function
# optionaly IOV of created payload can be set as
# basf2 arich/utility/scripts/ARICHImportPrametersToDB.py -- 3 0 3 -1
# for example for all runs of experiment 3
# arguments: 1 experimentLow, 2 runLow, 3 experimentHigh , 4 runHigh

import basf2 as b2
from ROOT.Belle2 import ARICHDatabaseImporter
import sys

argvs = sys.argv
argc = len(argvs)

# set local database folder
b2.use_local_database("localdb/database.txt", "localdb")

# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = b2.register_module('EventInfoSetter')
eventinfo.initialize()

# load gearbox for reading parameters from xml files (by default in "arich/data")
paramloader = b2.register_module('Gearbox')
paramloader.initialize()

main = b2.create_path()
main.add_module(eventinfo)
b2.process(main)

# and run the importer
dbImporter = ARICHDatabaseImporter()

# set IOV if desired (default IOV is 0,0,-1,-1)
if argc == 5:
    dbImporter.SetIOV(int(argvs[1]), int(argvs[2]), int(argvs[3]), int(argvs[4]))

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

# Import merger cooling bodies geometry
# from arich/data/ARICH-Merger-cooling_v2.xml

# dbImporter.importMergerCoolingGeo()

# Import simple cosmic test geometry (i.e. one aerogel tile + scintilators).
# Parameters are read from arich/data/ARICH-CosmicTest.xml

# dbImporter.importCosmicTestGeometry()

# Import global alignment parameters
# Parameters are read from arich/data/ARICH-GlobalAlignment.xml

# dbImporter.importGlobalAlignment()

# Import mirror alignment parameters
# Parameters are read from arich/data/ARICH-MirrorAlignment.xml

# dbImporter.importMirrorAlignment()

# Import aerogel tile alignment parameters
# Parameters are read from arich/data/ARICH-AeroTilesAlignment.xml

# dbImporter.importAeroTilesAlignment()
