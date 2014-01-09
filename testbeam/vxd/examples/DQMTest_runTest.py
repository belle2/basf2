#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
input = register_module('RootInput')
input.param('inputFileName', 'TBSimulation.root')

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', 'SVD-histo.root')  # File to save histograms

# Report progress of processing
progress = register_module('Progress')

# Load parameters from xml
gearbox = register_module('Gearbox')
# VXD (no Telescopes), and the real PCMAG magnetic field
gearbox.param('fileName', 'testbeam/vxd/FullVXDTB.xml')

# Create geometry
geometry = register_module('Geometry')
# No magnetic field for this test,
geometry.param('components', ['TB'])

# PXD/SVD clusterizer
PXDClust = register_module('PXDClusterizer')
PXDClust.param('TanLorentz', 0.)

SVDClust = register_module('SVDClusterizer')
SVDClust.param('TanLorentz_holes', 0.)
SVDClust.param('TanLorentz_electrons', 0.)

# VXDTF:
## parameters:
secSetup = ['testBeamMini6GeVSVD-moreThan1500MeV_SVD']
qiType = 'circleFit'
filterOverlaps = 'hopfield'
#
vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.INFO
vxdtf.logging.debug_level = 2
param_vxdtf = {
    'activateBaselineTF': 1,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'standardPdgCode': -11,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'killEventForHighOccupancyThreshold': 75,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': False,
    'qiSmear': False,
    'smearSigma': 0.000001,
    'GFTrackCandidatesColName': 'caTracks',
    'tuneCutoffs': 5,
    'activateDistanceXY': [False],
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
    'activateAnglesRZ': [False],
    'activateDeltaSlopeRZ': [False],
    'activateDistance2IP': [False],
    'activatePT': [False],
    'activateHelixFit': [False],
    'activateZigZagXY': [False],
    'activateDeltaPt': [True],
    'activateCircleFit': [False],
    'tuneCircleFit': [0.00000001],
    'tuneAngles3D': [-0.1],
    'tuneDistance3D': [-0.1],
    }
vxdtf.param(param_vxdtf)


trackfitter = register_module('GenFitter')
#trackfitter.logging.log_level = LogLevel.WARNING
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('UseClusters', True)


# SVD DQM module
svd_dqm = register_module('SVDDQM')

vxdtf_dqm = register_module('VXDTFDQM')

# Create paths
main = create_path()

# Add modules to paths
main.add_module(input)
main.add_module(histo)  # immediately after master module
main.add_module(progress)
main.add_module(gearbox)
main.add_module(geometry)
# main.add_module(PXDClust)
main.add_module(SVDClust)
main.add_module(vxdtf)
main.add_module(trackfitter)

main.add_module(svd_dqm)
main.add_module(vxdtf_dqm)

# Process events
process(main)
