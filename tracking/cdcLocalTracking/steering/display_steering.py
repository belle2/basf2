#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
############################################################################################################################

import sys
import os
from optparse import OptionParser

parser = OptionParser()

# parser.add_option("--trasan",dest="trasan",         action="store_true", default=False, help="use trasan")
parser.add_option('--mcTracking', dest='mcTracking', action='store_true',
                  default=False, help='use mcTracking')
parser.add_option('--localTracking', dest='localTracking', action='store_true'
                  , default=True, help='use localTracking')
parser.add_option('--conformalTracking', dest='conformalTracking',
                  action='store_true', default=False,
                  help='use conformalTracking')
parser.add_option('--legendreTracking', dest='legendreTracking',
                  action='store_true', default=False,
                  help='use legendreTracking')

parser.add_option('--input', dest='input', default='-999',
                  help='location of input file')

(options, args) = parser.parse_args()

print options

from basf2 import *

# Add the path of the svg display module to the search path of python
belle2_local_dir = os.environ['BELLE2_LOCAL_DIR']
cdcdisplay_module_path = os.path.join(belle2_local_dir, 'tracking',
                                      'cdcLocalTracking', 'python_code')
sys.path.append(cdcdisplay_module_path)
import cdcdisplay

set_log_level(LogLevel.INFO)

# if (options.seed != -999):
#    set_random_seed(options.seed)

# ---------------------------------------------------------------
# EvtGen
# boost all momenta to LAB system
# use specified user decay file
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/analysis/modules/B2Dpi/datafiles/BtoDpi.dec')

# specify number of events to be generated in job
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])  # we want to process nOfEvents events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')

# ---------------------------------------------------------------
# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking+PID detectors
# geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'])
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])

# ---------------------------------------------------------------
# simulation
g4sim = register_module('FullSim')
g4sim.logging.log_level = LogLevel.ERROR
# param_g4sim = {'RegisterOptics': 1, 'PhotonFraction': 0.3,
#               'TrackingVerbosity': 0}
# g4sim.param(param_g4sim)

# ---------------------------------------------------------------
# CDC digitizer
cdcDigitizer = register_module('CDCDigitizer')

# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# ----------------------------------------------------------------
# trasan = register_module('Trasan')
# trasan.param('DebugLevel', 10)
# trasan.param('GFTrackCandidatesColName', 'GFTrackCands')

# ---------------------------------------------------------------
cdclegendretracking = register_module('CDCLegendreTracking')

param_cdclegendretracking = {
    'GFTrackCandidatesColName': 'GFTrackCands',
    'Threshold': 10,
    'InitialAxialHits': 48,
    'MaxLevel': 10,
    'StepScale': 0.75,
    'Resolution StereoHits': 2.,
    'Reconstruct Curler': True,
    }

cdclegendretracking.param(param_cdclegendretracking)

# ---------------------------------------------------------------
# MC finder
mctrackfinder = register_module('TrackFinderMCTruth')
param_mctrackfinder = {
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'WhichParticles': ['primary'],
    'EnergyCut': 0.1,
    'Neutrals': 0,
    'GFTrackCandidatesColName': 'GFTrackCands',
    }

mctrackfinder.param(param_mctrackfinder)

# ---------------------------------------------------------------
# Local finder
cdclocaltracking = register_module('CDCLocalTracking')
# cdclocaltracking.logging.log_level = LogLevel.DEBUG

# ---------------------------------------------------------------
# Conformal finder
# cdcconformaltracking = register_module('CDCTracking')

# param_cdcconformaltracking = {'GFTrackCandidatesColName': 'GFTrackCands'}
# cdcconformaltracking.param(param_cdcconformaltracking)

# select which detectors you would like to use

# ---------------------------------------------------------------
# CDCSVGDisplayModule
svgdisplay = cdcdisplay.CDCSVGDisplayModule('/tmp')

svgdisplay.draw_wires = True  # and False
svgdisplay.draw_hits = True and False

# Objects only drawable with local tracking
svgdisplay.draw_mctracks = True and False
svgdisplay.draw_mcvertices = True and False
svgdisplay.draw_simhits = True and False
svgdisplay.draw_rlinfo = True and False
svgdisplay.draw_tof = True and False
svgdisplay.draw_clusters = True and False
svgdisplay.draw_segments = True and False
svgdisplay.draw_tangentsegments = True and False
svgdisplay.draw_segmenttriples = True and False
svgdisplay.draw_tracks = True and False
svgdisplay.draw_segmenttriple_trajectories = True and False
svgdisplay.draw_segment_trajectories = True and False

svgdisplay.draw_gftrackcands = True  # and False
svgdisplay.draw_gftrackcand_trajectories = True  # and False

svgdisplay.draw_superlayer_boundaries = True  # and False
svgdisplay.draw_interactionpoint = True  # and False

# ---------------------------------------------------------------
# input
input = register_module('RootInput')
input.param('inputFileName', options.input)

# ---------------------------------------------------------------
# Add all modules to the main path
main = create_path()

if options.input == '-999':
    main.add_module(eventinfosetter)
    main.add_module(evtgeninput)

    main.add_module(progress)

    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(g4sim)

    main.add_module(cdcDigitizer)
else:

    main.add_module(input)
    main.add_module(gearbox)

if options.mcTracking:
    main.add_module(mctrackfinder)
elif options.localTracking:
# elif (options.trasan):
#    main.add_module(trasan)
    main.add_module(cdclocaltracking)
elif options.conformalTracking:
    main.add_module(cdcconformaltracking)
elif options.legendreTracking:
    main.add_module(cdclegendretracking)

main.add_module(svgdisplay)

process(main)

# Print call statistics
print statistics

