#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script first runs clusterization and track finding on unpacked data.
# Found tracks are fitted using General Broken Lines, which produces
# a binary file for Millepede II alignment.
# Finally it tries to run Millepede II alignment (still to be debugged in TB real data conditions, be patient)
# NOTE: currently only SVD data is used. Once PXD will be in the beam, PXD will be added

import os
import sys

from basf2 import *
set_log_level(LogLevel.WARNING)

# ----------------------------
# Geometry
# ----------------------------

# Here you can turn magnetic field ON/OFF
# ... sets approprite geometry components and parameters of clusterizers
fieldOn = False

# Load Geometry module
gearbox = register_module('Gearbox')

# Here you have to specify the geometry for reconstruction
# Uncomment desired geometry file
gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v1.xml')  # Telescopes, daemon PXDs (air), SVD
# gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml') # Telescopes, PXD6-S (layer 2) and SVD

# This module constructs the geometry
# By specifing components, you can disable the magnetic field of PCMag
geometry = register_module('Geometry')
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# ----------------------------
# Clusterization
# ----------------------------

# SVD clusterizer module
SVDClust = register_module('SVDClusterizer')
if fieldOn:
    SVDClust.param('TanLorentz_holes', 0.052)
    SVDClust.param('TanLorentz_electrons', 0.)
else:
    SVDClust.param('TanLorentz_holes', 0.)
    SVDClust.param('TanLorentz_electrons', 0.)

# ----------------------------
# SVD data quality monitoring
# ----------------------------

# SVD DQM module
SVD_DQM = register_module('SVDDQM')

# ----------------------------
# Track finding
# ----------------------------

# VXDTF module
# Here you will have to specify calculated sector map to be used for track finding
# Currently we have just this sector map for TB. Jakob will prepare appropriate sector map
# for each setup

## parameters:
secSetup = ['testBeamMini6GeVSVD-moreThan1500MeV_SVD']  # SVD-only track finding
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

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

# ----------------------------
# Track fitting with GBL
# ----------------------------

# General Broken Lines fitting module
# This module first runs genfit DAF fitting to initialize genfit tracks
# and then pass tracks from track finding to the GBL interface for fitting.
# This module produces a binary file for Millepede II alignment
gbl = register_module('GBLfit')
gbl.logging.log_level = LogLevel.ERROR
# NOTE: This module produces a ROOT file "gbl.root" which contains
# results of GBL fitting, incl. residuals, pulls, p-values etc.
gbl.param('milleFileName', 'millefile.dat')
gbl.param('minNdf', 1)
gbl.param('pValueCut', 0.05)

# ----------------------------
# Millepede II alignment
# ----------------------------

# NOTE: you can always run Millepede manually from command line once binary file is produced:
# "pede steering_file_name"
# but in that case alignment results won't be translated into xml files

# Alignment with Millepede II.
# NOTE: this module outputs also a text file "constraints.txt" into execution directory
# You can link this file from your steering file to use geometry constrains for alignment
alignment = register_module('MillepedeIIalignment')
# You have to specifi MP2 steering file name
alignment.param('steeringFileName', 'steer.txt')

# ----------------------------
# Input / Output
# ----------------------------

# Add input module
input = register_module('RootInput')
# Here you have to set path to file with unpacked raw data (with digits)
input.param('inputFileName', '/path/to/unpacked/data/unpacked_data.root')
# input.param ( "inputFileName", "/mnt/win/basf2/release/testbeam/vxd/online/run74.root")

# Add output module
output = register_module('RootOutput')
output.param('outputFileName', 'fittedRun.root')
# NOTE: Trackfit objects in output file come from DAF, not GBL.
# GBL fit results are histogrammed in gbl.root file produced by GBLfit module

# ----------------------------
# Some info / helper modules
# ----------------------------

# Add Progress
progress = register_module('Progress')

# Add Elapsed Time
elapsed = register_module('ElapsedTime')
elapsed.param('EventInterval', 10000)

# Add event display module
display = register_module('Display')
# Use this parameter to display TB geometry (not Belle2 geometry extract)
display.param('fullGeometry', True)

# ----------------------------
# Path construction
# ----------------------------

# create the main path
main = create_path()
# Add input module
main.add_module(input)
# Add Gearbox and geometry
main.add_module(gearbox)
main.add_module(geometry)
# Add SVD clusterizer
main.add_module(SVDClust)
# Add SVD DQM module
# main.add_module(SVD_DQM)
# Add track finding
main.add_module(vxdtf)
# Add track finding DQM
# main.add_module(vxdtf_dqm)
# Add GBL track fitting
main.add_module(gbl)
# Add output module
main.add_module(output)
# Try to run Millepede II alignment ... will start at the end of basf2 execution
main.add_module(alignment)
# Add info modules and display
main.add_module(display)
main.add_module(progress)
main.add_module(elapsed)

# Run
process(main)
