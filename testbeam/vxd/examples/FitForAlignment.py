#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script refits a file produced by Digits2Tracks.py
# and also (despite its name) does the alignment.
#
# It is intented as next step after initial alignment is computed
# in Digits2Tracks.py. This alignment is here loaded and track fitting
# (and optionaly track finding) is performed again.
# Also the alignment is computed again, this time corrections should be small
#
# Sorry for mismatch of usual and Gear paths. Only geometry and masks use Gear paths

import sys
from basf2 import *
from setup_vxdtf import *
from alignment_tools import *

# Set the log level to show only warning, error and, fatal messages
set_log_level(LogLevel.WARNING)
release = str(os.getenv('BELLE2_LOCAL_DIR')) + '/'

# -------------------------------------------------
#          IMPORTANT PROCESSING SETTINGS
# -------------------------------------------------
# Starting with stage 1, each next stage takes
# results as previous one as input
STAGE = 1
# STAGE = 2
# WARNING: You must set correct mille binary name in MP2 steering
# Here only stage 1 and 2 are prepared

# Geometry file: _v1 for GeoID=0, _v2 for GeoID=1
geometry = 'testbeam/vxd/FullTelescopeVXDTB_v2.xml'  # Gear path
# geometry = 'testbeam/vxd/FullTelescopeVXDTB_v1.xml' # Gear path

# Custom alignment must be set at the very begining as it modifies main xml
# during execution of basf2
# NOTE: You need a separated (from main xml) file for playing with alignment
alignment = 'alignment_stage_' + str(STAGE) + '.xml'

# Turn ON/OFF 1T constant mag. field, affects: geometry/tracking, clustering, track finding
PCMAG_ON = True  # only 1T/0T supported this way

# If last alignment corrections were large, better to repeat track finding
# However, once you find "final" alignment, you should repeat the whole chain from Digits2Tracks.py
redo_trackfinding = True
# This sector map must reflect field ON/OFF and nominal beam energy ... look for it or create your own
sectormap = 'TB4GeV1TRun500VXD-moreThan1500MeV_VXD'
# sectormap = 'TB0T3GeVVXD-moreThan1500MeV_VXD'

# ------------- INPUT/OUTPUT SETTINGS ------------

# Input root file with merged digits + event data (nothing else)
input_root = 'Digits2Tracks.root'
# Output file with added clusters and tracks
output_root = 'FitForAlignment_stage_' + str(STAGE) + '.root'
# Complete DQM for digits/clusters/correlations/track finding/Kalman fitting...
output_dqm = 'DQM_FitForAlignment_stage_' + str(STAGE) + '.root'
# Output Mille binary file with data for alignment
alignment_binary = 'millefile_stage_' + str(STAGE) + '.dat'

# Often you do not want to run Millepede automatically. Then set this to False
do_alignment = True
# File with final alignment after MP2 correction (+ initial)
output_alignment = 'alignment_stage_' + str(STAGE + 1) + '.xml'
# File with alignment steering
# NOTE: You have to link the alignment_binary (or more such files) from the steering file
alignment_steering = 'steer_stage_' + str(STAGE) + '.txt'
# This is just intermediate step btw. MP2 and sum of alignment ... just translated millepede.res file
output_alignment_corrections = 'alingment_corrections_stage_' + str(STAGE) \
    + 'to' + str(STAGE + 1) + '.xml'

# ------------------------------------------------
# ------------------------------------------------

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', output_dqm)

# Set the custom alignment
set_alignment = SetAlignment(release + 'data/' + geometry, alignment)

# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('fileName', geometry)

# Create geometry
geometry = register_module('Geometry')
if PCMAG_ON:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# input data
input = register_module('RootInput')
input.param('inputFileName', input_root)

# Clean-up collections (remove genfit stuff), we need just this:
branchNames = [
    'EventMetaData',
    'PXDClusters',
    'PXDClustersToPXDDigits',
    'PXDDigits',
    'SVDClusters',
    'SVDClustersToSVDDigits',
    'SVDDigits',
    'TelEventInfo',
    'TelDigits',
    'TelClusters',
    'TelClustersToTelDigits',
    ]
# ... and caTracks if we do not want to repeat track finding (=hit matching + mom. seed)
if not redo_trackfinding:
    branchNames.append('caTracks')
input.param('branchNames', branchNames)

# Track finding by VXDTF
if redo_trackfinding:
    if PCMAG_ON:
        VXDTF = setup_vxdtf1T('caTracks', [sectormap])
    else:
        VXDTF = setup_vxdtf('caTracks', [sectormap])

# Track finding DQM
vxdtfdqm = register_module('VXDTFDQM')
vxdtfdqm.param('GFTrackCandidatesColName', 'caTracks')

# Track fitting wit GENFIT2
trackfitter = register_module('GenFitter')
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('StoreFailedTracks', True)
trackfitter.param('NMinIterations', 3)
trackfitter.param('NMaxIterations', 8)
trackfitter.logging.log_level = LogLevel.ERROR

# Track fitting DQM
tfdqm = register_module('TrackfitDQM')
tfdqm.param('GFTrackCandidatesColName', 'caTracks')

# GBL fitting -> produces binary data for alignment
gbl = register_module('GBLfit')
gbl.param('GFTrackCandidatesColName', 'caTracks')
# Suppose first alignment step is finished, apply chi squared cut
# Default value is 50. (used also in MP2)
gbl.param('chi2Cut', 50.0)
gbl.param('FilterId', 'Kalman')
gbl.param('internalIterations', '')
gbl.param('milleFileName', alignment_binary)
gbl.logging.log_level = LogLevel.ERROR

# Millepede II alignment ... Millepede can be run without basf2
mp2 = register_module('MillepedeIIalignment')
mp2.param('resultXmlFileName', output_alignment_corrections)
mp2.param('steeringFileName', alignment_steering)

# Root output
dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', output_root)
# Clean-up collections
dataWriter.param('branchNames', [
    'EventMetaData',
    'PXDClusters',
    'PXDClustersToPXDDigits',
    'PXDDigits',
    'SVDClusters',
    'SVDClustersToSVDDigits',
    'SVDDigits',
    'TelEventInfo',
    'TelDigits',
    'TelClusters',
    'TelClustersToTelDigits',
    'caTracks',
    'Tracks',
    'TrackFitResults',
    'GF2Tracks',
    'caTracksToGF2Tracks',
    ])

# Display progress of processing
progress = register_module('Progress')

# Event display (comment in path if not needed)
display = register_module('Display')
display.param('fullGeometry', True)
display.param('GFTrackCandidatesColName', 'caTracks')
display.param('options', 'DHMPS')

# -----------------------------------------------
#               Path construction
# -----------------------------------------------

main = create_path()
main.add_module(input)
main.add_module(histo)
main.add_module(set_alignment)
main.add_module(gearbox)
main.add_module(geometry)
if redo_trackfinding:
    main.add_module(VXDTF)
main.add_module(vxdtfdqm)
main.add_module(trackfitter)
main.add_module(tfdqm)
main.add_module(gbl)
if do_alignment:
    main.add_module(mp2)
# main.add_module(display)
main.add_module(dataWriter)
main.add_module(progress)

process(main)

# ------------------------------------------------
#  Sum initial and computed alignment parameters
# ------------------------------------------------
# You can run this together with Millepede from command line:
#
# Create/copy/use existing steering file, set correct path to binary file.
# Then just call pede:
#
#      pede steering_file.txt
#
# The result will be in millepede.res file (in execution path)
# Now just start basf2 as python interpreter (with no arguments)
# and type:
#
#      from alignment_tools import *
#      write_alignment( sum_xmltxt_alignment( 'your_initial_alignment.xml', 'millepede.res' ) )
#
if do_alignment:
    write_alignment(sum_xmltxt_alignment(alignment_xml_path=alignment,
                    alignment_txt_path='millepede.res'), output_alignment)

# Or in this case (if you run MP2-module, you will have
# the corrections as xml), you can use this:

