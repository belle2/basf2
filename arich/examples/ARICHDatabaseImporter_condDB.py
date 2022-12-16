#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import ROOT
from ROOT.Belle2 import ARICHDatabaseImporter
import os
from optparse import OptionParser
from fnmatch import fnmatch

parser = OptionParser()
parser.add_option('-c', '--class', dest='whatclass', default='none', help='')
parser.add_option('-x', '--i-e', dest='importexport', default='export', help='')
parser.add_option('-t', '--hvtest', dest='hvtest', default='no', help='')
parser.add_option('-o', '--aopRootF', dest='aopRootF', default='ARICH_AerogelOpticalProperties.root', help='')
(options, args) = parser.parse_args()
ie = options.importexport
ieClass = options.whatclass
hvtest = options.hvtest
aopRootF = options.aopRootF

home = os.environ['BELLE2_LOCAL_DIR']

# More information about use_local_database please find here:
# https://software.belle2.org/development/sphinx/framework/doc/index-03-framework.html?highlight=use_local_database#basf2.use_local_database
b2.use_local_database()
# use_local_database("./ARICHdata/centraldb/database.txt", "./ARICHdata/centraldb/", False, LogLevel.ERROR, False)
# use_local_database("./ARICH_db_Test/centraldb/database.txt", "", False, LogLevel.ERROR, False)
# use_local_database("test_database.txt", "test_payloads")
# use_local_database("test_database.txt", "test_payloads")
# use use_central_database for uploading data to PNNL
# use_central_database("ARICHdata", LogLevel.ERROR)
# use_central_database("ARICH_db_Test", LogLevel.ERROR)
# use_central_database("development", LogLevel.ERROR)
#

# EventInfoSetter is only needed to register EventMetaData in the Datastore to
# get rid of an error message with gearbox
eventinfo = b2.register_module('EventInfoSetter')
eventinfo.initialize()

main = b2.create_path()
main.add_module(eventinfo)

# create a gearbox module to read the aerogel data so it can be used
# it needs to open just the aerogel/AerogelData.xml which includes all other xml
# files in that folder. What we do is that we set the backend to point to that
# folder so that gearbox can find all files in there.

if(ie == "import"):
    paramloader = b2.register_module('Gearbox')
    pathname = 'file://%s/data/AllData/' % (os.getcwd())
    paramloader.param('backends', [pathname])
    paramloader.param('fileName', 'ArichData.xml')
    paramloader.initialize()

    # create a std::vector<string>
    rootFilesHapdQA = ROOT.vector('string')()
    # and add the files we want to read
    xdirHapdQA = '%s/hapdQA/' % (os.getcwd())
    for path, subdirs, files in os.walk(xdirHapdQA):
        for name in files:
            if fnmatch(name, "*.root"):
                rootFilesHapdQA.push_back(os.path.join(path, name))

    # create a std::vector<string>
    rootFilesHapdQE = ROOT.vector('string')()
    # and add the files we want to read
    # QE can be downloaded from http://research.kek.jp/group/arich/internal/pixel_map/
    xdirHapdQE = '%s/hapdQE/' % (os.getcwd())
    for path, subdirs, files in os.walk(xdirHapdQE):
        for name in files:
            if fnmatch(name, "*.root"):
                rootFilesHapdQE.push_back(os.path.join(path, name))

    # create a std::vector<string>
    rootFilesAsics = ROOT.vector('string')()
    txtFilesAsics = ROOT.vector('string')()
    # create path to directory
    number = 'UY'
    xdir = ('%s/asicData' + number + '/') % (os.getcwd())
    # and add the files we want to read
    for path, subdirs, files in os.walk(xdir):
        for name in files:
            if fnmatch(name, "*.root"):
                rootFilesAsics.push_back(os.path.join(path, name))
            if fnmatch(name, "*.txt"):
                txtFilesAsics.push_back(os.path.join(path, name))

    # create a std::vector<string>
    rootFilesFebTest = ROOT.vector('string')()
    # and add the files we want to read
    xdirFebTest = '%s/febTest/' % (os.getcwd())
    for path, subdirs, files in os.walk(xdirFebTest):
        for name in files:
            if fnmatch(name, "*.root"):
                rootFilesFebTest.push_back(os.path.join(path, name))
    mypath = '%s/moduleTest/modules/' % (os.getcwd())

if(ie == 'importAerogelInfoOnly'):
    paramloader = b2.register_module('Gearbox')
    pathname = '/home/b-lab050/KEK/xmlarichdata/data/aerogel_xml_ver3_R/'
    paramloader.param('backends', [pathname])
    paramloader.param('fileName', 'AerogelData.xml')
    paramloader.initialize()

if(ie == 'importAerogelRayleighScatteringFit'):
    paramloader = b2.register_module('Gearbox')
    pathname = '/home/b-lab050/KEK/xmlarichdata/xmlData/aerogel_xml_ver3_L/'
    paramloader.param('backends', [pathname])
    paramloader.param('fileName', 'aerogel_xml_ver3_L_fit.xml')
    paramloader.initialize()

b2.process(main)

if(ie == 'importAerogelInfoOnly'):
    dbImporter = ARICHDatabaseImporter()
    dbImporter.importAerogelInfo("Right")

if(ie == 'importAerogelRayleighScatteringFit'):
    dbImporter = ARICHDatabaseImporter()
    dbImporter.importAeroRayleighScatteringFit("Left")


# and run the importer
if(ie == 'import'):
    dbImporter = ARICHDatabaseImporter(rootFilesHapdQA, rootFilesAsics, txtFilesAsics, rootFilesHapdQE, rootFilesFebTest)
    if(ieClass == 'aerogelInfo'):
        dbImporter.importAerogelInfo()
    if(ieClass == 'aerogelMap'):
        dbImporter.importAerogelMap()
    if(ieClass == 'aerogelTilesInfo'):
        dbImporter.importAeroTilesInfo()
    if(ieClass == 'hapdQA'):
        dbImporter.importHapdQA()
    if(ieClass == 'hapdQE'):
        dbImporter.importHapdQE()
    if(ieClass == 'hapdChipInfo'):
        dbImporter.importHapdChipInfo()
    if(ieClass == 'hapdInfo'):
        dbImporter.importHapdInfo()
    if(ieClass == 'hapd'):
        dbImporter.importHapdQA()
        dbImporter.importHapdQE()
        dbImporter.importHapdChipInfo()
        dbImporter.importHapdInfo()
    if(ieClass == 'asicInfo'):
        dbImporter.importAsicInfo()
    if(ieClass == 'febTest'):
        dbImporter.importFebTest()
    if(ieClass == 'febInfo'):
        dbImporter.importFEBoardInfo()
    if(ieClass == 'moduleTest'):
        dbImporter.importModuleTest(mypath, hvtest)
    if(ieClass == 'sensorInfo'):
        dbImporter.importSensorModuleInfo()
    if(ieClass == 'sensorMap'):
        dbImporter.importSensorModuleMap()

if(ie == 'importAerogelTilesInfo'):
    dbImporter = ARICHDatabaseImporter()
    if(ieClass == 'aerogelTilesInfo'):
        dbImporter.importAeroTilesInfo()

if(ie == 'export'):
    dbImporter = ARICHDatabaseImporter()
    if(ieClass == 'aerogelInfo'):
        dbImporter.exportAerogelInfo()
    if(ieClass == 'aerogelMap'):
        dbImporter.exportAerogelMap()
    if(ieClass == 'aerogelTilesInfo'):
        dbImporter.printAeroTileInfo()
    if(ieClass == 'dumpAerogelOpticalProperties'):
        # dbImporter.dumpAerogelOpticalProperties()
        # dbImporter.dumpAerogelOpticalProperties("ARICH_AerogelOpticalProperties.root")
        dbImporter.dumpAerogelOpticalProperties(aopRootF)
        com = 'root -l ' + aopRootF + ' ' + home + '/arich/utility/scripts/plotTestARICHAerogelHist.C'
        os.system(com)
    if(ieClass == 'hapdQA'):
        dbImporter.exportHapdQA()
    if(ieClass == 'hapdQE'):
        dbImporter.exportHapdQE()
    if(ieClass == 'hapdInfo'):
        dbImporter.exportHapdInfo()
    if(ieClass == 'hapdChipInfo'):
        dbImporter.exportHapdChipInfo()
    if(ieClass == 'hapd'):
        dbImporter.exportHapdQA()
        dbImporter.exportHapdQE()
        dbImporter.exportHapdInfo()
    if(ieClass == 'asicInfo'):
        dbImporter.exportAsicInfo()
    if(ieClass == 'febTest'):
        dbImporter.exportFebTest()
    if(ieClass == 'febInfo'):
        dbImporter.exportFEBoardInfo()
    if(ieClass == 'moduleTest'):
        dbImporter.exportModuleTest(hvtest)
    if(ieClass == 'sensorMap'):
        dbImporter.exportSensorModuleMap()
    if(ieClass == 'all'):
        dbImporter.exportAll()


# print parameters used in simulation/reconstruction software
# dbImporter.printSimulationPar()
# dbImporter.printModulesInfo()
# dbImporter.printChannelMapping()
# dbImporter.printGeometryConfig()
# dbImporter.dumpQEMap(True)
# dbImporter.dumpModuleNumbering()


# simple example that shows how to read data from database and use it
# dbImporter.getBiasVoltagesForHapdChip("KA0167")

# simple example that shows how to extract aerogel recontruction data from database
# dbImporter.getMyParams("A165")
