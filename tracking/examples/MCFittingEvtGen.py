#!/usr/bin/env python
# -*- coding: utf-8 -*-

###########################################################################################################################
#
# This steering file creates the Belle II detector geometry,
# and perfoms the simulation with EVTGen as generator and MC based track finding and fitting.
#
# EvtMetaGen and EvtMetaInfo generates and shows event meta data (see example in the framework package).
# Gearbox and Geometry are used to create the Belle2 detector geometry.
# The generator used in this example is geant4 particle gun (see example in the simulation or generator package).
# FullSim performs the full simulation.

# CDCDigi creates the detector response in the CDC for the simulated Hits.
# For the PXD and SVD currently the TrueHits are used (created directly by the sensitive detector), will be replaced by realistic clusters later on.

# MCTrackFinder creates relations between MCParticles and CDCHits/PXDTrueHits/SVDTrueHits produced by it.
# GenFitter fits the found MCTracks and created two track collections: GFTracks (Genfit class) and Tracks (class with helix parametrization)
#
# For details about module parameters just type > basf2 -m .
#
############################################################################################################################

import os
from basf2 import *

# register necessary modules
evtmetagen = register_module('EvtMetaGen')

# generate one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [1])

evtmetainfo = register_module('EvtMetaInfo')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking detectors
# to simulate the whole detector included in BelleII.xml, comment the next line out
geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
               ])
# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

# if the files are not in your data/generators folder, copy them there from externals/v...../evtgen
evtgeninput.param('DECFile', os.path.join(basf2datadir, 'generators/DECAY.DEC'
                  ))
evtgeninput.param('pdlFile', os.path.join(basf2datadir, 'generators/evt.pdl'))
evtgeninput.param('boost2LAB', True)

# simulation
g4sim = register_module('FullSim')
# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigi')

# use one gaussian with resolution of 0.01 in the digitizer (to simplify the fitting)
param_cdcdigi = {'Fraction': 1, 'Resolution1': 0.01, 'Resolution2': 0.0}
cdcDigitizer.param(param_cdcdigi)

# find MCTracks
mctrackfinder = register_module('MCTrackFinder')

# select which detectors you would like to use
param_mctrackfinder = {'UseCDCHits': 1, 'UseSVDHits': 1, 'UsePXDHits': 1}

# select which particles to use, here as example: use only particles which had created hits in the CDC, have an energy > 0.1 and are charged
param_mctrackfinder = {'WhichParticles': 3, 'EnergyCut': 0.1, 'Neutrals': 0}
mctrackfinder.param(param_mctrackfinder)

# fitting
cdcfitting = register_module('GenFitter')

# fit the tracks with one iteration of Kalman filter
param_cdcfitting = {
    'StoreFailedTracks': 0,
    'mcTracks': 1,
    'FilterId': 0,
    'NIterations': 1,
    'ProbCut': 0.001,
    }
cdcfitting.param(param_cdcfitting)

# output
output = register_module('SimpleOutput')
output.param('outputFileName', 'MCFittingEvtGenOutput.root')

# create paths
main = create_path()

# add modules to paths
main.add_module(evtmetagen)
main.add_module(evtmetainfo)

main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)

main.add_module(cdcDigitizer)

main.add_module(mctrackfinder)
main.add_module(cdcfitting)
main.add_module(output)

# Process events
process(main)

print statistics
