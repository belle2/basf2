#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script (despite its name) contains the whole
# processing chain including alignment.
# It is intented as direct next step after digit merging (see eutel/examples).
# Telescopes are clusterized but the track finder ignores them so far
#
# This script starts with custom alignment file
# and finishes with resulting file with sum initial and computed alignment
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

# Geometry file: _v1 for GeoID=0, _v2 for GeoID=1
geometry_file = 'testbeam/vxd/FullTelescopeVXDTB_v2.xml'  # Gear path
# geometry = 'testbeam/vxd/FullTelescopeVXDTB_v1.xml' # Gear path

# Custom alignment must be set at the very begining as it modifies main xml
# during execution of basf2
# NOTE: You need a separated (from main xml) file for playing with alignment
alignment = release + 'testbeam/vxd/data/nominal_alignment.xml'
# alignment = '/mnt/win/577_alignment.xml'

# Turn ON/OFF 1T constant mag. field, affects: geometry/tracking, clustering, track finding
PCMAG_ON = False  # only 1T/0T supported this way

# This sector map must reflect field ON/OFF and nominal beam energy ... look for it or create your own
# sectormap = 'TB4GeV1TRun500VXD-moreThan1500MeV_VXD'
# sectormap = 'TB0T3GeVVXD-moreThan1500MeV_VXD'
sectormap = \
    ['TB4GeVNoMagnetNoAlignedSource2014May21PXDSVDTEL-moreThan1500MeV_TELPXDSVD'
     ]
# Masking for sensors
pxd_mask = 'testbeam/vxd/data/PXD-IgnoredPixelsList.xml'  # Gear path
svd_mask = 'testbeam/vxd/data/SVD-IgnoredStripsList.xml'  # Gear path

# ------------- INPUT/OUTPUT SETTINGS ------------

# Input root file with merged digits + event data (nothing else)
# input_root = '/mnt/win/alignment/SetBestCandidate607_1.root'
input_root = 'SimulatedDigits.root'

# Output file with added clusters and tracks
output_root = 'Digits2Tracks.root'
# Complete DQM for digits/clusters/correlations/track finding/Kalman fitting...
output_dqm = 'DQM_Digits2Tracks.root'
# Output Mille binary file with data for alignment
alignment_binary = 'millefile_stage_0.dat'

# Often you do not want to run Millepede automatically. Then set this to False
do_alignment = True
# File with final alignment after MP2 correction (+ initial)
output_alignment = 'alignment_stage_1.xml'
# File with alignment steering
# NOTE: You have to link the alignment_binary (or more such files) from the steering file
alignment_steering = 'steer_stage_0.txt'
# This is just intermediate step btw. MP2 and sum of alignment ... just translated millepede.res file
output_alignment_corrections = 'alingment_corrections_stage_0to1.xml'

# ------------------------------------------------
# ------------------------------------------------

# Histogram manager immediately after master module
histo = register_module('HistoManager')
histo.param('histoFileName', output_dqm)

# Set the custom alignment
set_alignment = SetAlignment(release + 'data/' + geometry_file, alignment)

# Load parameters from xml
gearbox = register_module('Gearbox')
gearbox.param('fileName', geometry_file)

# Create geometry
geometry = register_module('Geometry')
if PCMAG_ON:
    geometry.param('components', ['MagneticField', 'TB'])
else:
    geometry.param('components', ['TB'])

# input data
input = register_module('RootInput')
input.param('inputFileName', input_root)
input.param('excludeBranchNames', ['caTracks'])

# Masking in fact:
PXDDSort = register_module('PXDDigitSorter')
PXDDSort.param('ignoredPixelsListName', pxd_mask)

# PXD clusterizer
PXDClust = register_module('PXDClusterizer')
if PCMAG_ON:
    PXDClust.param('TanLorentz', 0.1625)  # value scaled from 0.25 for 1.5T to 0.975T
else:
    PXDClust.param('TanLorentz', 0.)

# PXD DQM module
PXD_DQM = register_module('PXDDQM')

# SVD Digit sorter ... masking
SVDSort = register_module('SVDDigitSorter')
SVDSort.param('mergeDuplicates', False)
# No gearbox conventions, VERBATIM path!
SVDSort.param('ignoredStripsListName', svd_mask)

# SVD clusterizer
SVDClust = register_module('SVDClusterizer')
if PCMAG_ON:
    SVDClust.param('TanLorentz_holes', 0.052)  # value scaled from 0.08 for 1.5T to 0.975T
    SVDClust.param('TanLorentz_electrons', 0.)
else:
    SVDClust.param('TanLorentz_holes', 0.)
    SVDClust.param('TanLorentz_electrons', 0.)
# SVDClust.param('applyTimeWindow', True)
# SVDClust.param('triggerTime', -20.0)
# SVDClust.param('acceptanceWindowSize', 20.0)

# SVD DQM module
svd_dqm = register_module('SVDDQM')

# EUDET clusterizer
TelClust = register_module('TelClusterizer')
TelClust.param('Clusters', 'TelClusters')

# TEL DQM module
tel_dqm = register_module('TelDQM')
tel_dqm.param('Clusters', 'TelClusters')

# TB DQM module adds correlations between telescopes and nearest VXD sensors.
telvxd_dqm = register_module('TelxVXD')

# Track finding by VXDTF
if PCMAG_ON:
    VXDTF = setup_vxdtf1T('caTracks')
else:
    VXDTF = setup_vxdtf('caTracks')

# Track finding DQM
vxdtfdqm = register_module('VXDTFDQM')
vxdtfdqm.param('GFTrackCandidatesColName', 'caTracks')

# Track fitting wit GENFIT2
trackfitter = register_module('GenFitterVXDTB')
trackfitter.param('GFTrackCandidatesColName', 'caTracks')
trackfitter.param('FilterId', 'Kalman')
trackfitter.param('StoreFailedTracks', True)
trackfitter.param('NMinIterations', 3)
trackfitter.param('NMaxIterations', 8)
trackfitter.logging.log_level = LogLevel.DEBUG

# Track fitting DQM
tfdqm = register_module('TrackfitDQM')
tfdqm.param('GFTrackCandidatesColName', 'caTracks')

# GBL fitting -> produces binary data for alignment
gbl = register_module('GBLfit')
gbl.param('GFTrackCandidatesColName', 'caTracks')
# This removes cut on hit Chi squared. Use it if geometry is heavily misaligned
# Default value is 50.    ... use this once first alignment step is finished
gbl.param('chi2Cut', 50.)
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
main.add_module(PXDDSort)
main.add_module(PXDClust)
main.add_module(PXD_DQM)
main.add_module(SVDSort)
main.add_module(SVDClust)
main.add_module(svd_dqm)
main.add_module(TelClust)
main.add_module(tel_dqm)
main.add_module(telvxd_dqm)
main.add_module(VXDTF)
main.add_module(vxdtfdqm)
main.add_module(trackfitter)
main.add_module(tfdqm)

# main.add_module(gbl)
# if do_alignment:
#    main.add_module(mp2)

main.add_module(display)
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
# if do_alignment:
#    write_alignment(sum_xmltxt_alignment(alignment_xml_path=alignment,
#                    alignment_txt_path='millepede.res'), output_alignment)

# Or in this case (if you run MP2-module, you will have
# the corrections as xml), you can use this:

