#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
input = register_module('RootInput')
input.param('inputFileName', 'TBSimulation.root')

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQM-VXD-histo.root')  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v1.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

# PXD rawhit sorter: convert PXDRawHits to PXDDigits. .
PXDSort = register_module('PXDRawHitSorter')
PXDSort.param('mergeDuplicates', False)
PXDSort.param('mergeFrames', False)
# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
PXDClust.param('TanLorentz', 0.)
#SVD sorter
SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)
# Use the list of ignored strips. _Verbatim_ path, this is not Gear.
SVDSort.param('ignoredStripsListName', \
    'testbeam/vxd/data/SVD-IgnoredStripsList.xml')
SVDClust = register_module('SVDClusterizer')
SVDClust.param('TanLorentz_holes', 0.)
SVDClust.param('TanLorentz_electrons', 0.)

TelClust = register_module('TelClusterizer')
TelClust.param('Clusters', 'TelClusters')

# PXD DQM module
pxd_dqm = register_module('PXDDQM')

# SVD DQM module
svd_dqm = register_module('SVDDQM')

# TEL DQM module
tel_dqm = register_module('TelDQM')
tel_dqm.param('Clusters', 'TelClusters')

# VXDTF:
## parameters:
secSetup = ['TB6GeVNoMagnetSVD-moreThan1500MeV_SVD']
qiType = 'circleFit'
filterOverlaps = 'hopfield'  # hopfield
#
vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.INFO
vxdtf.logging.debug_level = 1
param_vxdtf = {
    'activateBaselineTF': 1,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'standardPdgCode': -11,
    'artificialMomentum': 5.,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'killEventForHighOccupancyThreshold': 75,
    'highOccupancyThreshold': 85,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': False,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
    'tuneCutoffs': 4,
    'activateDistanceXY': [False],
    'activateDistanceZ': [True],
    'activateDistance3D': [True],
    'activateAngles3DHioC': [False],
    'activateAnglesXYHioC': [False],
    'activateDeltaSlopeRZHioC': [False],
    'activateDistance2IPHioC': [False],
    'activatePTHioC': [False],
    'activateHelixFitHioC': [False],
    'activateDeltaPtHioC': [False],
    'activateDeltaDistance2IPHioC': [False],
    'activateAngles3D': [True],
    'activateAnglesXY': [False],
    'activateAnglesRZ': [True],
    'activateDeltaSlopeRZ': [False],
    'activateDistance2IP': [False],
    'activatePT': [False],
    'activateHelixFit': [False],
    'activateZigZagXY': [False],
    'activateDeltaPt': [False],
    'activateCircleFit': [True],
    'tuneCircleFit': [0],
    'tuneDistanceZ': [1.2],
    }
vxdtf.param(param_vxdtf)

trackfitter = register_module('GenFitter')
# trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('UseClusters', True)

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

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
main.add_module(pxd_dqm)
main.add_module(svd_dqm)
main.add_module(tel_dqm)
main.add_module(vxdtf)
main.add_module(trackfitter)
main.add_module(vxdtf_dqm)

# Process events
process(main)
