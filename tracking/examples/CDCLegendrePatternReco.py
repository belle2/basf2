#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
############################################################################################################################

import sys
import os
from optparse import OptionParser

parser = OptionParser()

parser.add_option(
    '-l',
    '--threshold',
    dest='threshold',
    type=int,
    default='10',
    help='threshold for peak finder',
    )
parser.add_option(
    '-m',
    '--maxLevel',
    dest='maxLevel',
    type=int,
    default='10',
    help='maximal level for fastHough algorithm',
    )
parser.add_option(
    '-i',
    '--initalAxial',
    dest='initialAxial',
    type=int,
    default='30',
    help='intial number of axial hits in steppedHough algorithm',
    )
parser.add_option(
    '-c',
    '--stepScale',
    dest='stepScale',
    type=float,
    default='0.75',
    help='step size for steppedHough algorithm',
    )
parser.add_option(
    '-b',
    '--resolutionStereo',
    dest='resolutionStereo',
    type=float,
    default='2.',
    help='resolution of stereo hits',
    )
parser.add_option(
    '-n',
    '--NoCurler',
    dest='NotRecCurler',
    action='store_true',
    default=False,
    help='reconstruct curler',
    )

parser.add_option('--mcTracking', dest='mcTracking', action='store_true',
                  default=False, help='use mcTracking')
parser.add_option('--Fitting', dest='Fitting', action='store_true',
                  default=False, help='avoid fitting of tracks')
parser.add_option('--seed', dest='seed', type=int, default='-999',
                  help='random seed')

parser.add_option('--input', dest='input', default='-999',
                  help='location of input file')

(options, args) = parser.parse_args()

print options

from basf2 import *

set_log_level(LogLevel.INFO)

if options.seed != -999:
    set_random_seed(options.seed)

# ---------------------------------------------------------------
# EvtGen
# boost all momenta to LAB system
# use specified user decay file
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)
evtgeninput.param('userDECFile', os.environ['BELLE2_LOCAL_DIR']
                  + '/analysis/modules/B2Dpi/datafiles/BtoDpi.dec')

# specify number of events to be generated in job
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('EvtNumList', [1])  # we want to process nOfEvents events
evtmetagen.param('RunList', [1])  # from run number 1
evtmetagen.param('ExpList', [1])  # and experiment number 1

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
g4sim.param('StoreAllSecondaries', True)  # this is need for the MCTrackFinder to work correctly
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

# ---------------------------------------------------------------
# Add CDC background hits
# It doesn't work yet, so it's set to 0
cdcBackground = register_module('CDCSimpleBackground')
cdcBackground.param('BGLevelHits', 0.0)
cdcBackground.param('BGLevelClusters', 0.0)

# ---------------------------------------------------------------
cdctracking = register_module('CDCLegendreTracking')
param_cdctracking = {
    'GFTrackCandidatesColName': 'GFTrackCands',
    'Threshold': options.threshold,
    'InitialAxialHits': options.initialAxial,
    'MaxLevel': options.maxLevel,
    'StepScale': options.stepScale,
    'Resolution StereoHits': options.resolutionStereo,
    'Reconstruct Curler': not options.NotRecCurler,
    }
cdctracking.param(param_cdctracking)

mctrackfinder = register_module('MCTrackFinder')

# select which detectors you would like to use
# select which detectors you would like to use and select which particles to use: primary particles
param_mctrackfinder = {
    'WhichParticles': ['primary'],
    'EnergyCut': 0.1,
    'Neutrals': 0,
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    }
mctrackfinder.param(param_mctrackfinder)

mctrackfinder = register_module('MCTrackFinder')

# ---------------------------------------------------------------
# match the found track candidates with MCParticles
mcmatching = register_module('CDCMCMatching')

# select the correct collection for the matching
param_mcmatching = {'GFTrackCandidatesColName': 'GFTrackCands'}
mcmatching.param(param_mcmatching)

# ---------------------------------------------------------------
# Track fitting is performed with GenFit
cdcfitting = register_module('GenFitter')

# set correct collection name as input and custom collection names as output
# select DAF instead of Kalman as Filter
# set the pdg hypothesis to the simulated one, if you want to fit with different pdg hypothesises, just state them all in the PDGCodes list
param_cdcfitting = {
    'GFTracksColName': 'GFTracks',
    'TracksColName': 'Tracks',
    'PDGCodes': [211],
    'FilterId': 1,
    'NIterations': 1,
    'ProbCut': 0.001,
    }

cdcfitting.param(param_cdcfitting)

# ---------------------------------------------------------------
# input
input = register_module('RootInput')
input.param('inputFileName', options.input)

# ---------------------------------------------------------------
# Add all modules to the main path
main = create_path()

if options.input == '-999':
    main.add_module(evtmetagen)
    main.add_module(evtgeninput)

    main.add_module(progress)

    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(g4sim)

    main.add_module(cdcDigitizer)
else:

    # main.add_module(cdcBackground)

    main.add_module(input)
    main.add_module(gearbox)

if options.mcTracking:
    main.add_module(mctrackfinder)
else:
    main.add_module(cdctracking)

main.add_module(mcmatching)

if options.Fitting:
    main.add_module(cdcfitting)

process(main)

# Print call statistics
print statistics

