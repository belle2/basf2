#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# For TB DESY 2016, Peter Kodys

import os
from basf2 import *
# This is testbeam/vxd/scripts/setup_vxdtf.py
from setup_vxdtf import *

# fieldOn = True
fieldOn = True  # Turn field on or off (changes geometry components and digi/clust params)

# flag to use if the PXD is in place and should be used in the Reconstruction
havePXD = False
havePXD = True

# Register modules
input = register_module('RootInput')
# if fieldOn:
#    input.param('inputFileName', 'TBSimulationWBfield.root')
# else:
input.param('inputFileName', 'TBSimulation.root')

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'TB2016-OnLineDQM-VXD-histo.root')  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
# if havePXD:
#     gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml')
# else:
#     gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v1.xml')
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB2016.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
# if fieldOn:
#    geometry.param('components', ['MagneticField', 'TB'])
if not fieldOn:
    # To turn off magnetic field:
    geometry.param('excludedComponents', ['MagneticField'])
# else:
#     geometry.param('components', ['TB'])

# PXD rawhit sorter: convert PXDRawHits to PXDDigits. .
PXDSort = register_module('PXDRawHitSorter')
PXDSort.param('mergeDuplicates', False)
PXDSort.param('mergeFrames', False)
# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
# PXDClust.param('TanLorentz', 0.)
# SVD sorter
SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)
# Use the list of ignored strips. _Verbatim_ path, this is not Gear.
SVDClust = register_module('SVDClusterizer')
# SVDClust.param('TanLorentz_holes', 0.)
# SVDClust.param('TanLorentz_electrons', 0.)

TelClust = register_module('TelClusterizer')
TelClust = register_module('TelClusterizer')
# TelClust.param('Clusters', 'TelClusters')

# VXD on line DQM module
vxd_dqm = register_module('VXDDQMOnLine')
vxd_dqm.param('SaveOtherHistos', 1)
vxd_dqm.param('SwapPXD', 0)
vxd_dqm.param('Reduce1DCorrelHistos', 1)

trackfitterTel = register_module('GenFitterVXDTB')
trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
# trackfitter.param('GFTrackCandidatesColName', 'caTracks')
# trackfitter.param('FilterId', 'Kalman')
# trackfitter.param('UseClusters', True)

filterOverlaps = 'hopfield'

if fieldOn:
    # SVD and PXD sec map
    # secSetup = ['TB2016Test8Feb2016MagnetOnPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD:
    secSetup = ['TB2016Test8Feb2016MagnetOnSVD-moreThan1500MeV_SVD']
    qiType = 'circleFit'  # circleFit
else:
    # To turn off magnetic field:
    # SVD and PXD sec map:
    # secSetup = ['TB2016Test8Feb2016MagnetOffPXDSVD-moreThan1500MeV_PXDSVD']
    # only SVD
    secSetup = ['TB2016Test8Feb2016MagnetOffSVD-moreThan1500MeV_SVD']
    qiType = 'straightLine'  # straightLine

vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.DEBUG
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
    'GFTrackCandidatesColName': 'TrackCands',
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


# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

# Save output of simulation
output = register_module('RootOutput')
output.param('outputFileName', 'DQMTest_output.root')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(histo)  # immediately after master module
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(PXDSort)
main.add_module(PXDClust)
main.add_module(SVDSort)
main.add_module(SVDClust)
main.add_module(TelClust)
main.add_module(vxd_dqm)
main.add_module(output)

# Process events
process(main)
