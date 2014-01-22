#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This python script contains the most complete chain of processing.
# WARNING: It is not intended to be fast (you can comment-out some modules to make it fast)
# It starts processing from unpacked Raw SVD (PXD and EUTELs will be added soon).
# It contains:
# - geometry loading (incl. B field if requested)
# - input of unpacked data
# - basic filtering of SVD digits (now mainly because of SVD6)
# - track finding using Jakob's VXDTF
# - fitting with genfit2's Kalman filter
# - DQM of:
#   - SVD
#   - Track finding
#   - Track fitting
#   - Beam parameters (e.g. for alignment of the setup with respect to the beam)
# - fitting with General Broken Lines with binary output for Millepede II (MP2)
# - (computation of alignment parameters with MP2 ... commented out as it needs adjusting MP2 steering ... will be soon)
# - progreass
# - output of ROOT objects, output of DQM histograms

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
# gearbox.param('fileName', 'testbeam/vxd/FullTelescopeVXDTB_v2.xml') # Telescopes squeezed, PXD6-S (layer 2) and SVD

# This module constructs the geometry
# By specifing components, you can disable the magnetic field of PCMag
geometry = register_module('Geometry')
if fieldOn:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# ----------------------------
# Data Input
# ----------------------------

# Add input module
input = register_module('RootInput')
# Here you have to set path to file with unpacked raw data (with digits)
input.param('inputFileName',
            '/mnt/win/basf2/release/testbeam/vxd/online/run220.root')

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

# ------------------------------
# SVD Digits Filtering / Masking
# ------------------------------

# Add super stupid module, which masks 1/3 of SVD6:-)
# You MUST add the filter to each sensor (defaultly it accepts everything from 0 to 768)
SVDFilterSVD6 = register_module('SVDDigitFilter')
# these two are default settings for collection names.
# You MUST unpack your data with param svdDigitListName equal to "SVDDigitsNotFiltered"
SVDFilterSVD6.param('inputDigits', 'SVDDigitsNotFiltered')
SVDFilterSVD6.param('outputDigits', 'SVDDigits')
# Now brute-force masking of all SVDs:
SVDFilterSVD6.param('sensorID', '6.1.6')
SVDFilterSVD6.param('maskVupTo', 10)
SVDFilterSVD6.param('maskUupTo', 8)
SVDFilterSVD6.param('maskVupTo', 150)
SVDFilterSVD6.param('maskUfrom', 600)
SVDFilterSVD6.param('maskVfrom', 470)
SVDFilterSVD6.param('maskStripsU', [
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    128,
    129,
    400,
    401,
    402,
    403,
    404,
    ])
SVDFilterSVD6.param('maskStripsV', [])

SVDFilterSVD5 = register_module('SVDDigitFilter')
SVDFilterSVD5.param('sensorID', '5.1.5')
SVDFilterSVD5.param('maskVupTo', 10)
SVDFilterSVD5.param('maskUupTo', 20)
SVDFilterSVD5.param('maskVupTo', 150)
SVDFilterSVD5.param('maskUfrom', 600)
SVDFilterSVD5.param('maskVfrom', 470)
SVDFilterSVD5.param('maskStripsU', [])
SVDFilterSVD5.param('maskStripsV', [
    110,
    111,
    112,
    113,
    114,
    300,
    301,
    302,
    303,
    304,
    305,
    306,
    307,
    308,
    309,
    ])

SVDFilterSVD4 = register_module('SVDDigitFilter')
SVDFilterSVD4.param('sensorID', '4.1.4')
SVDFilterSVD4.param('maskUupTo', 8)
SVDFilterSVD4.param('maskVupTo', 150)
SVDFilterSVD4.param('maskUfrom', 600)
SVDFilterSVD4.param('maskVfrom', 470)
SVDFilterSVD4.param('maskStripsU', [])
SVDFilterSVD4.param('maskStripsV', [])

SVDFilterSVD3 = register_module('SVDDigitFilter')
SVDFilterSVD3.param('sensorID', '3.1.3')
SVDFilterSVD3.param('maskUupTo', 16)
SVDFilterSVD3.param('maskVupTo', 150)
SVDFilterSVD3.param('maskUfrom', 620)
SVDFilterSVD3.param('maskVfrom', 470)
SVDFilterSVD3.param('maskStripsU', [])
SVDFilterSVD3.param('maskStripsV', [])

# ----------------------------
# Track finding
# ----------------------------

# VXDTF module
# Here you will have to specify calculated sector map to be used for track finding
# Currently we have just this sector map for TB. Jakob will prepare appropriate sector map
# for each setup

# parameters:
secSetup = ['testBeamMini6GeVSVD-moreThan1500MeV_SVD']  # SVD-only track finding
qiType = 'circleFit'
filterOverlaps = 'hopfield'
#
vxdtf = register_module('VXDTF')
vxdtf.logging.log_level = LogLevel.ERROR
vxdtf.logging.debug_level = 2
param_vxdtf = {
    'activateBaselineTF': 1,
    'tccMinState': [2],
    'tccMinLayer': [3],
    'standardPdgCode': -11,
    'sectorSetup': secSetup,
    'calcQIType': qiType,
    'killEventForHighOccupancyThreshold': 77,
    'highOccupancyThreshold': 85,
    'cleanOverlappingSet': False,
    'filterOverlappingTCs': filterOverlaps,
    'TESTERexpandedTestingRoutines': False,
    'qiSmear': False,
    'smearSigma': 1e-06,
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

# ----------------------------
# Track fitting with GENFIT2
# ----------------------------

genfit = register_module('GenFitter')
genfit.param('GFTrackCandidatesColName', 'caTracks')
# Use Kalman filter with reference track
genfit.param('FilterId', 'Kalman')

# -------------------------------
# DQM Modules
# -------------------------------

# Histogram manager must be immediately after master module in the path
histo = register_module('HistoManager')
histo.param('histoFileName', 'DQM_run220.root')  # File to save histograms

# SVD data quality monitoring
SVD_DQM = register_module('SVDDQM')

# VXDTF DQM module
vxdtf_dqm = register_module('VXDTFDQM')

# Track fitting with Kalman DQM
tfdqm = register_module('TrackfitDQM')
tfdqm.param('GFTrackCandidatesColName', 'caTracks')

# Beam DQM
BeamDQM1 = register_module('BeamDQM')
BeamDQM1.param('fieldFileName', 'field_plane_3.txt')
# BeamDQM1.param('dataFileName', 'scan_plane_3.txt')
BeamDQM1.param('sensorID', '3.1.3')
BeamDQM1.param('nBinX', 10)
BeamDQM1.param('nBinY', 10)
BeamDQM1.param('minX', -0.5)
BeamDQM1.param('minY', -0.6)
BeamDQM1.param('maxX', 0.8)
BeamDQM1.param('maxY', 1.0)

BeamDQM2 = register_module('BeamDQM')
BeamDQM2.param('fieldFileName', 'field_plane_4.txt')
# BeamDQM2.param('dataFileName', 'scan_plane_4.txt')
BeamDQM2.param('sensorID', '4.1.4')
BeamDQM2.param('nBinX', 10)
BeamDQM2.param('nBinY', 10)
BeamDQM2.param('minX', -0.5)
BeamDQM2.param('minY', -0.6)
BeamDQM2.param('maxX', 0.8)
BeamDQM2.param('maxY', 0.6)

BeamDQM3 = register_module('BeamDQM')
BeamDQM3.param('fieldFileName', 'field_plane_5.txt')
# BeamDQM3.param('dataFileName', 'scan_plane_5.txt')
BeamDQM3.param('sensorID', '5.1.5')
BeamDQM3.param('nBinX', 10)
BeamDQM3.param('nBinY', 10)
BeamDQM3.param('minX', -0.5)
BeamDQM3.param('minY', -0.6)
BeamDQM3.param('maxX', 0.8)
BeamDQM3.param('maxY', 0.6)

BeamDQM4 = register_module('BeamDQM')
BeamDQM4.param('fieldFileName', 'field_plane_6.txt')
# BeamDQM4.param('dataFileName', 'scan_plane_6.txt')
BeamDQM4.param('sensorID', '6.1.6')
BeamDQM4.param('nBinX', 10)
BeamDQM4.param('nBinY', 10)
BeamDQM4.param('minX', -0.5)
BeamDQM4.param('minY', -0.6)
BeamDQM4.param('maxX', 0.8)
BeamDQM4.param('maxY', 0.6)

# ----------------------------
# Track fitting with GBL
# ----------------------------

# General Broken Lines fitting module
# This module first runs genfit DAF fitting to initialize genfit tracks
# and then passes tracks from track finding to the GBL interface for fitting.
# This module produces a binary file for Millepede II alignment
gbl = register_module('GBLfit')
gbl.logging.log_level = LogLevel.ERROR
# NOTE: This module produces a ROOT file "gbl.root" which contains
# results of GBL fitting, incl. residuals, pulls, p-values etc.
gbl.param('milleFileName', 'millefile_run220.dat')
gbl.param('GFTrackCandidatesColName', 'caTracks')
gbl.param('minNdf', 1)
gbl.param('pValueCut', 0.)

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
alignment.param('steeringFileName',
                '/mnt/win/basf2/release/testbeam/vxd/examples/steer.txt')

# ---------------------------------------
# Data Output of collections to ROOT file
# ---------------------------------------

# Add output module
output = register_module('RootOutput')
output.param('outputFileName', 'output_run220.root')

# ------------------------------------------
# Some info / helper modules / Event Display
# ------------------------------------------

# Add Progress
progress = register_module('Progress')

# Add Elapsed Time and processing event rate
elapsed = register_module('ElapsedTime')
elapsed.param('EventInterval', 100)

# Add event display module
display = register_module('Display')
# Use this parameter to display TB geometry (not Belle2 geometry extract)
display.param('fullGeometry', True)
display.param('GFTrackCandidatesColName', 'caTracks')
display.param('options', 'DHMPS')

# ----------------------------
# Path construction
# ----------------------------

# create the main path
main = create_path()
# hitogram manager for DQM modules
main.add_module(histo)
# Add input module
main.add_module(input)
# Add Gearbox and geometry
main.add_module(gearbox)
main.add_module(geometry)
# Filter SVD digits in all planes to new collection
main.add_module(SVDFilterSVD3)
main.add_module(SVDFilterSVD4)
main.add_module(SVDFilterSVD5)
main.add_module(SVDFilterSVD6)
# Add SVD clusterizer
x = register_module('SVDDigitSorter')
main.add_module(x)
main.add_module(SVDClust)
# Add track finding
main.add_module(vxdtf)
# Add GBL track fitting
main.add_module(genfit)
# Add SVD DQM module
main.add_module(SVD_DQM)
# Add track finding DQM
main.add_module(vxdtf_dqm)
# Add TrackfitDQM module
main.add_module(tfdqm)
# Add BeamDQM
main.add_module(BeamDQM1)
main.add_module(BeamDQM2)
main.add_module(BeamDQM3)
main.add_module(BeamDQM4)
# Add output module
main.add_module(output)
# Add GBL fitting with output for alignment
# main.add_module(gbl)
# Try to run Millepede II alignment ... will start at the end of basf2 execution
# main.add_module(alignment)
# Add info modules and display
main.add_module(display)
main.add_module(progress)
main.add_module(elapsed)

# Run
process(main)
print statistics
