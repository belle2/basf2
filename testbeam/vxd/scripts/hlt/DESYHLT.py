#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This steering file will simulate several testbeam events, reconstruct
# and fit tracks, and display each event.
import os
from basf2 import *
from subprocess import call
import datetime
from ROOT import Belle2


argvs = sys.argv
argc = len(argvs)

# ## setup of the most important parts for the VXDTF ###
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)
filterOverlaps = 'hopfield'

# Random number seed
seed = 1  # 1, 5, 6
set_random_seed(seed)

# Log level
# set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)
# set_log_level(LogLevel.DEBUG)

# Local database access
reset_database()
# use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
databasefile = Belle2.FileSystem.findFile("data/framework/database.txt")
use_local_database(databasefile, os.path.dirname(databasefile), True)

# Gearbox
gearbox = register_module('Gearbox')
# use simple testbeam geometry
gearbox.param('fileName',
              'testbeam/vxd/FullVXDTB2016.xml')

# Geometry
geometry = register_module('Geometry')
# only the tracking detectors will be simulated. Makes the example much faster
if fieldOn:
    # SVD and PXD sec map
    # secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD:
    secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
    # To turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])
    # SVD and PXD sec map:
    # secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD
    secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

# Unpacker
SVDUNPACK = register_module('SVDUnpacker')

# Digit sorter and Clusterizer
# The digit sorter also suppresses data from masked strips.
# It is not needed for simulation.
SVDSORT = register_module('SVDDigitSorter')
SVDCLUST = register_module('SVDClusterizer')

# VXDTF
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

# Track Fitter
trackfitter = register_module('GenFitter')
trackfitter.logging.log_level = LogLevel.FATAL
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('GFTracksColName', 'gfTracks')
trackfitter.param('FilterId', 'Kalman')
# trackfitter.param('StoreFailedTracks', True)
trackfitter.param('StoreFailedTracks', False)
# trackfitter.param('FilterId', 'simpleKalman')
trackfitter.param('UseClusters', True)

# Genfit Extrapolator
setupGenfit = register_module('SetupGenfitExtrapolation')

##
# ROI FINDING
##

roiprod = register_module('PXDDataReduction')
roiprod.logging.log_level = LogLevel.ERROR
param_roiprod = {
    'gfTrackListName': 'gfTracks',
    'PXDInterceptListName': 'PXDIntercepts',
    'ROIListName': 'ROIs',
    'sigmaSystU': 0.02,
    'sigmaSystV': 0.02,
    'numSigmaTotU': 10,
    'numSigmaTotV': 10,
    'maxWidthU': 0.5,
    'maxWidthV': 0.5,
}
roiprod.param(param_roiprod)

# payload
roipayload = register_module('ROIPayloadAssembler')
param_roiPayloadAssembler = {
    'ROIListName': 'ROIs',
    'ROIpayloadName': 'ROIpayload'}
roipayload.param(param_roiPayloadAssembler)

# Dummy RoIs Generator
roiprod_debug = register_module('ROIGenerator')
roiprod_debug.param('ROIListName', 'debugROIs')
roiprod_debug.param('MinU', 0)
roiprod_debug.param('MaxU', 479)
roiprod_debug.param('MinV', 0)
roiprod_debug.param('MaxV', 191)
roiprod_debug.param('TrigDivider', 5)
roiprod_debug.param('Sensor', 2)

# payload
roipayload_debug = register_module('ROIPayloadAssembler')
roipayload_debug.set_name('ROIPayloadAssembler_debug')
param_roiPayloadAssembler_debug = {
    'ROIListName': 'debugROIs',
    'ROIpayloadName': 'ROIpayload'}
roipayload_debug.param(param_roiPayloadAssembler_debug)

# DQM modules

# HistoManager
histoman = register_module('HistoManager')
histoman.param('histoFileName', 'histofile_std.root')

# SVD DQM module
svddqm = register_module('SVDDQM3')

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')
vxdtf_dqm.param('GFTrackCandidatesColName', 'caTracks')
vxdtf_dqm.logging.log_level = LogLevel.ERROR
# vxdtf_dqm.logging.log_level = LogLevel.DEBUG
# vxdtf_dqm.logging.debug_level = 1

# TF analyzer
analyzer = register_module('TFAnalizer')
analyzer.logging.log_level = LogLevel.ERROR
# analyzer.logging.debug_level = 1
param_analyzer = {'printExtentialAnalysisData': False, 'caTCname': 'caTracks'}
analyzer.param(param_analyzer)

# TrackFit DQM
trackfit_dqm = register_module('TrackfitDQM')
trackfit_dqm.param('GFTrackCandidatesColName', 'caTracks')

# Input and Output

# Input module
input = register_module('SeqRootInput')
# input.param('inputFileName', argvs[1] )
input.param('inputFileName', 'RoISimTB2016_SVD.sroot')

# Raw2Ds
# input = register_module('Raw2Ds')
# input.param ('RingBufferName', 'DRBUF')

# Output module
output = register_module('RootOutput')
# output.param('outputFileName', argvs[2] )
output.param('outputFileName', 'RoITrkTB2016_SVD.root')

# Progress
progress = register_module('Progress')

# Print Collections
pcol = register_module('PrintCollections')

# Create paths
main = create_path()

# Add modules to paths

main.add_module(input)
main.add_module(histoman)

# Geometry etc.
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(setupGenfit)

# Unpack SVD
main.add_module(SVDUNPACK)
main.add_module(SVDSORT)
main.add_module(SVDCLUST)

# Track finding and fitting
main.add_module(vxdtf)
main.add_module(trackfitter)

# ROI finding
main.add_module(roiprod)  # standard ROI production
main.add_module(roipayload)  # payload for standard ROI production
# main.add_module(roiprod_debug) # dummy ROI production
# main.add_module(roipayload_debug) # payload for dummy ROI production

# DQM
main.add_module(svddqm)
main.add_module(vxdtf_dqm)
# main.add_module(analyzer)
main.add_module(trackfit_dqm)

main.add_module(output)
main.add_module(progress)
# main.add_module(pcol)

# Limit streaming objects
set_streamobjs(['EventMetaData', 'RawSVDs', 'RawFTSWs',
                'ROIpayload', 'ROIs', 'debugROIs',
                'HLTTag', 'ProcessStatistics',
                'caTracks', 'caTracksTogfTracks', 'gfTracks'])
#                'SVDClusters', 'SVDDigits', 'SVDClustersToSVDDigits'] )

# Run it

# set_nprocesses(2)
process(main)

# Statistics
print(statistics)
