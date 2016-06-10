#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will
import os
from sys import argv
from time import time
from basf2 import *
from beamparameters import add_beamparameters

from subprocess import call
import datetime
from ROOT import Belle2


# magnet on or off
magnetOn = True

# if true a root file is expected as input
useRootInput = True

# set the type of track fit used
filterOverlaps = 'hopfield'
qiType = 'straightLine'
momentum = 4.0
if magnetOn:
    qiType = 'kalman'
    momentum = -1.0  # if momentum is set for B field on one observes a peak in the momentum distribution


# if true alignment parameters will be applied to the trackfitting and space point transformation
useAlignment = True  # True


# first argument of the input
inputFileName = sys.argv[1]
outputDir = sys.argv[2]
outputFileName = os.path.basename(inputFileName)
outputFileName = outputDir + '/' + outputFileName.replace('.root', '_out.root')
print('using the following input file: ',  inputFileName)
print('writing output to: ',  outputFileName)

# security check to not overwrite files!
if os.path.exists(outputFileName):
    print('Error output file already exists: ', outputFileName)
    print('Delete it before proceeding!')
    sys.exit()


# nominal sector maps (ie. no layer excluded)
# magnet off svd and pxd
secMaps_MagOff_PXDSVD = {
  'default': 'TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD',
  'noL3':    'TB2016Test8Feb2016MagnetOff_noLayer3_PXDSVD-moreThan1500MeV_PXDSVD',
  'noL4':    'TB2016Test8Feb2016MagnetOff_noLayer4_PXDSVD-moreThan1500MeV_PXDSVD',
  'noL5':    'TB2016Test8Feb2016MagnetOff_noLayer5_PXDSVD-moreThan1500MeV_PXDSVD',
  'noL6':    'TB2016Test8Feb2016MagnetOff_noLayer6_PXDSVD-moreThan1500MeV_PXDSVD'
}

# magnet off only svd
secMaps_MagOff_SVD = {
  'default': 'TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD',
  'noL3':    'TB2016Test8Feb2016MagnetOff_noLayer3_SVD-moreThan1500MeV_SVD',
  'noL4':    'TB2016Test8Feb2016MagnetOff_noLayer4_SVD-moreThan1500MeV_SVD',
  'noL5':    'TB2016Test8Feb2016MagnetOff_noLayer5_SVD-moreThan1500MeV_SVD',
  'noL6':    'TB2016Test8Feb2016MagnetOff_noLayer6_SVD-moreThan1500MeV_SVD'
}

# magnet on svd and pxd
secMaps_MagOn_PXDSVD = {
  'default': 'TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD',
  'noL3':    'TB2016Test8Feb2016MagnetOn_noLayer3_PXDSVD-moreThan1500MeV_PXDSVD',
  'noL4':    'TB2016Test8Feb2016MagnetOn_noLayer4_PXDSVD-moreThan1500MeV_PXDSVD',
  'noL5':    'TB2016Test8Feb2016MagnetOn_noLayer5_PXDSVD-moreThan1500MeV_PXDSVD',
  'noL6':    'TB2016Test8Feb2016MagnetOn_noLayer6_PXDSVD-moreThan1500MeV_PXDSVD'
}

# magnet on only svd
secMaps_MagOn_SVD = {
  'default': 'TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD',
  'noL3':    'TB2016Test8Feb2016MagnetOn_noLayer3_SVD-moreThan1500MeV_SVD',
  'noL4':    'TB2016Test8Feb2016MagnetOn_noLayer4_SVD-moreThan1500MeV_SVD',
  'noL5':    'TB2016Test8Feb2016MagnetOn_noLayer5_SVD-moreThan1500MeV_SVD',
  'noL6':    'TB2016Test8Feb2016MagnetOn_noLayer6_SVD-moreThan1500MeV_SVD'
}

set_log_level(LogLevel.ERROR)
# set_random_seed(initialValue)
set_random_seed(12345)

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
# eventinfosetter.param('evtNumList', [numEvents])

eventinfoprinter = register_module('EventInfoPrinter')

gearbox = register_module('Gearbox')
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB2016.xml')


evtgeninput = register_module('EvtGenInput')
evtgeninput.logging.log_level = LogLevel.WARNING

geometry = register_module('Geometry')
if not magnetOn:
    geometry.param('excludedComponents', ['MagneticField'])

geometry.param('excludedComponents', ['Scintilators'])


# TESTING ONLY: take constant magnetic field
# geometry.param('excludedComponents', ['MagneticField'] )
# geometry.param('additionalComponents', ['MagneticFieldConstant'] )


g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)


#
# start to loop over layers to create an own analysis setup for each layer ###
#
# select sector map; differentiate between magnet on/off and pxd+svd and only svd
if magnetOn:
    secSetup = secMaps_MagOn_SVD['default']
else:  # magnet off
    secSetup = secMaps_MagOff_SVD['default']


# vxdtf to find the tracks, input is the store array with reduced clusters!!
trackColName = 'caTracks'
vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.WARNING  # DEBUG
vxdtf.logging.debug_level = 2
param_vxdtf = {  # normally we don't know the particleID, but in the case of the testbeam,
    # we can expect (anti-?)electrons...
    # True
    # 'artificialMomentum': 5., ## uncomment if there is no magnetic field!
    # 7
    # 'activateDistance3D': [False],
    # 'activateDistanceZ': [True],
    # 'activateAngles3D': [False],
    # 'activateAnglesXY': [True],  #### noMagnet
    # ### withMagnet
    # 'activateAnglesRZHioC': [True], #### noMagnet
    # ### withMagnet r51x
    # True
    # 'useAlignmentFromDB': useAlignment, # True,
    'GFTrackCandidatesColName': trackColName,  # 'TrackCands',
    'sectorSetup': secSetup,
    # 'activateBaselineTF': 1,
    'debugMode': 0,
    # 'tccMinState': [2],
    # 'tccMinLayer': [3],
    'reserveHitsThreshold': [0.],
    'highestAllowedLayer': [6],
    'standardPdgCode': -11,
    'artificialMomentum': momentum,
    'calcQIType': qiType,
    'killEventForHighOccupancyThreshold': 500,
    'highOccupancyThreshold': 111,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'tuneCutoffs': 0.51,
    'activateDistanceXY': [False],
    'activateDistance3D': [True],
    'activateDistanceZ': [False],
    'activateSlopeRZ': [False],
    'activateNormedDistance3D': [False],
    'activateAngles3D': [True],
    'activateAnglesXY': [False],
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [False],
    'activateDistance2IP': [False],
    'activatePT': [False],
    'activateHelixParameterFit': [False],
    'activateAngles3DHioC': [True],
    'activateAnglesXYHioC': [True],
    'activateAnglesRZHioC': [False],
    'activateDeltaSlopeRZHioC': [False],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [False],
    'activateHelixParameterFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateZigZagXY': [False],
    'activateZigZagRZ': [False],
    'activateDeltaPt': [False],
    'activateCircleFit': [False],
}
vxdtf.param(param_vxdtf)


GFTracksColName = 'gfTracks'
trackfitter = register_module('GenFitterVXDTB')  # this version supports telescope clusters!
# trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.ERROR  # DEBUG
trackfitter.param('GFTrackCandidatesColName', trackColName)
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('GFTracksColName', GFTracksColName)
trackfitter.param('UseClusters', True)
trackfitter.param('PDGCodes', [11])

pxdeffi = register_module('PXDEfficiency')
pxdeffi.param('distCut', 0.05)  # distance cut wrt. the track fit in cm
pxdeffi.param('pxdclustersname', '')
pxdeffi.param('pxddigitsname', '')
pxdeffi.param('tracksname', 'gfTracks')
pxdeffi.param('useAlignment', True)
pxdeffi.param('writeTree', True)


setupGenfit = register_module('SetupGenfitExtrapolation')


eventCounter = register_module('EventCounter')
eventCounter.logging.log_level = LogLevel.INFO
eventCounter.param('stepSize', 25)

histo = register_module('HistoManager')
# histo.param('histoFileName', 'spacePointMatcherHists.root')  # File to save histograms
histo.param('histoFileName', outputFileName)  # File to save histograms

# Root input
roinput = register_module('RootInput')
roinput.param('inputFileName', inputFileName)
roinput.param('branchNames', ['EventMetaData', 'RawFTSWs', 'RawSVDs', 'RawPXDs'])
# roinput = register_module('SeqRootInput')

# Create paths
main = create_path()


# Local database access
reset_database()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))

# this is working:
# databasefile = Belle2.FileSystem.findFile("data/framework/database.txt")
# use_local_database(databasefile, os.path.dirname(databasefile), True)

# to use external alignment
use_database_chain()
# use_local_database(Belle2.FileSystem.findFile("/mnt/sdb1/home/thomas/belle2/releases/data/framework/database.txt"), "", True)
# used this one before 5.5.2016
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"), "", True)
use_local_database(Belle2.FileSystem.findFile("/mnt/sdb1/home/thomas/belle2/releases/head/data/framework/database.txt"), "", True)
use_local_database(Belle2.FileSystem.findFile('/mnt/sdb1/home/thomas/belle2/releases/head/aligne_tadeas/database.txt'), "", True)

# Add modules to paths
main.add_module(histo)

if not useRootInput:
    main.add_module(particlegun)

if useRootInput:
    main.add_module(roinput)

# Unpacker
SVDUNPACK = register_module('SVDUnpacker')
SVDUNPACK.param('xmlMapFileName', Belle2.FileSystem.findFile('data/testbeam/vxd/TB_svd_mapping.xml'))
SVDUNPACK.param('FADCTriggerNumberOffset', 1)
SVDUNPACK.param('shutUpFTBError', 10)


PXDSHIFT = register_module('PXDTriggerShifter')
PXDUNPACK = register_module('PXDUnpacker')
PXDUNPACK.param('RemapFlag', True)
PXDUNPACK.param('RemapLUT_IF_OB', Belle2.FileSystem.findFile('data/testbeam/vxd/LUT_IF_OB.csv'))
PXDUNPACK.param('RemapLUT_IB_OF', Belle2.FileSystem.findFile('data/testbeam/vxd/LUT_IB_OF.csv'))

# Digitizer
SVDDIGISORTER = register_module('SVDDigitSorter')
# tell it which strips to ignore
# "bad" strips estimated by eye on run 329 (outside beam also excluded)
# SVDDIGISORTER.param('ignoredStripsListName', 'data/testbeam/vxd/SVD-IgnoredStripsList-run329.xml')

PXDDIGISORTER = register_module('PXDRawHitSorter')

# Clusterizer
SVDCLUST = register_module('SVDClusterizer')
if magnetOn:
    SVDCLUST.param('TanLorentz_holes', 0.)  # 0.052
    SVDCLUST.param('TanLorentz_electrons', 0.)
else:
    SVDCLUST.param('TanLorentz_holes', 0.)  # value scaled from 0.08 for 1.5T to 0.975T
    SVDCLUST.param('TanLorentz_electrons', 0.)

PXDCLUST = register_module('PXDClusterizer')

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)

# when running over the root files the svd cluster already exist!
main.add_module(SVDUNPACK)
main.add_module(SVDDIGISORTER)
main.add_module(SVDCLUST)

main.add_module(PXDSHIFT)
main.add_module(PXDUNPACK)
main.add_module(PXDDIGISORTER)
main.add_module(PXDCLUST)


main.add_module(eventCounter)

main.add_module(vxdtf)
main.add_module(trackfitter)

main.add_module(pxdeffi)

# print the list of modules
modList = main.modules()
print("\nList of used module")
for modItem in modList:
    print(modItem.type())


# Process events
process(main)


print('Event Statistics :')
print(statistics)
