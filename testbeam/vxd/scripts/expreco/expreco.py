#! /usr/bin/env python

# EXPRESS RECO script for the 2016 testbeam
#
# Includes VXD reconstruction, VXDTF, tracking and DQMs.
# March 10, 2016

import os
import sys

from basf2 import *

set_log_level(LogLevel.ERROR)

argvs = sys.argv
argc = len(argvs)

# print argvs[1]
# print argvs[2]
# print argvs[3]
# print argvs[4]
# print argc

# Log level
set_log_level(LogLevel.INFO)

# create the main path
main = create_path()

# #####
# Input
# #####
input = register_module("SeqRootInput")
input.param("inputFileName", "test.global.0001.000138.sroot")
main.add_module(input)

# #####
# Histo
# #####
hman = register_module("HistoManager")
main.add_module(hman)

# #######
# B-field
# #######

fieldOn = True  # Turn field on or off (changes geometry components)

# ##
# DB
# ##

reset_database()
databasefile = Belle2.FileSystem.findFile("data/framework/database.txt")
use_local_database(databasefile, os.path.dirname(databasefile), True)

# ########
# Geometry
# ########

gearbox = register_module('Gearbox')
# simple testbeam geometry
gearbox.param('fileName',
              'testbeam/vxd/FullVXDTB2016.xml')

geometry = register_module('Geometry')
if not fieldOn:
    # Turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])

main.add_module(gearbox)
main.add_module(geometry)


# ###
# PXD
# ###

# Unpacker
pxd_unpack = register_module("PXDUnpacker")
main.add_module(pxd_unpack)

# Digitizer
pxd_sort = register_module("PXDDigitSorter")
main.add_module(pxd_sort)

# Clusterizer
pxd_clust = register_module("PXDClusterizer")
main.add_module(pxd_clust)

# PXDRawDQM
pxd_monitor = register_module("PXDRawDQM")
main.add_module(pxd_monitor)

# PXDDQM
pxd_dqm = register_module('PXDDQM')
main.add_module(pxd_dqm)

# ###
# SVD
# ###

# Unpacker
svd_unpack = register_module("SVDUnpacker")
main.add_module(svd_unpack)

# Digitizer
svd_sort = register_module("SVDDigitSorter")
main.add_module(svd_sort)

# Clusterizer
svd_clust = register_module("SVDClusterizer")
main.add_module(svd_clust)

# Add SVDDQM
svd_dqm = register_module('SVDDQM3')
main.add_module(svd_dqm)

# #####
# VXDTF
# #####

filterOverlaps = 'hopfield'
if fieldOn:
    # SVD and PXD sec map
    secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD:
    # secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
    # SVD and PXD sec map:
    secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD
    # secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.ERROR
vxdtf.logging.debug_level = 2
# calcQIType:
# Supports 'kalman', 'circleFit' or 'trackLength.
# 'circleFit' has best performance at the moment

# filterOverlappingTCs:
# Supports 'hopfield', 'greedy' or 'none'.
# 'hopfield' has best performance at the moment
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
    'activateBaselineTF': 1,
    'debugMode': 0,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'reserveHitsThreshold': [0.],
    'highestAllowedLayer': [6],
    'standardPdgCode': -11,
    'artificialMomentum': 3,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'killEventForHighOccupancyThreshold': 500,
    'highOccupancyThreshold': 111,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': True,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
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

main.add_module(vxdtf)

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')
vxdtf_dqm.param('GFTrackCandidatesColName', 'caTracks')
vxdtf_dqm.logging.log_level = LogLevel.ERROR
main.add_module(vxdtf_dqm)

# TF analyzer
analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.ERROR
# analyzer.logging.debug_level = 1
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)
main.add_module(analyzer)

# ###########
# TrackFitter
# ###########

trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.FATAL
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('GFTracksColName', 'gfTracks')
trackfitter.param('FilterId', 'Kalman')
# trackfitter.param('StoreFailedTracks', True)
trackfitter.param('StoreFailedTracks', False)
# trackfitter.param('FilterId', 'simpleKalman')
trackfitter.param('UseClusters', True)

main.add_module(trackfitter)

# TrackFit DQM
trackfit_dqm = register_module('TrackfitDQM')
trackfit_dqm.param('GFTrackCandidatesColName', 'caTracks')

# ######
# Output
# ######

output = register_module('RootOutput')
output.param('outputFileName', argvs[2])
# output.param('outputFileName', 'Expreco_output.root')

# ####
# MISC
# ####

# Add Progress
progress = register_module("Progress")
main.add_module(progress)

# Add Elapsed Time
elapsed = register_module("ElapsedTime")
elapsed.param("EventInterval", 10000)
main.add_module(elapsed)

# Run
process(main)
