#!/usr/bin/env python
# -*- coding: utf-8 -*-

#############################################################################
#
# This steering file creates the Belle II detector geometry, and perfoms the
# simulation and MC based track finding and fitting with General Broken Lines
#
# It also uses display to demonstrate its capability to visualize tracks
# fitted with GBL (using GBLfit Module)
#
##############################################################################
##

import os
from basf2 import *

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')

# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [100])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

# simulate only tracking detectors to simulate the whole detector included in
# BelleII.xml:
# geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
#                ])

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')
evtgeninput.param('boost2LAB', True)

# simulation
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
g4sim.param('SecondariesEnergyCut', 0.)
g4sim.param('trajectoryStore', 3)
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MCTracks
track_finder_mc_truth = register_module('TrackFinderMCTruth')

# select which detectors you would like to use
param_track_finder_mc_truth = {  # select which particles to use: primary particles
    'UseCDCHits': 1,
    'UseSVDHits': 1,
    'UsePXDHits': 1,
    'UseClusters': False,
    'WhichParticles': ['primary'],
}
track_finder_mc_truth.param(param_track_finder_mc_truth)

# fitting
fitting = register_module('GBLfit')
fitting.param('UseClusters', False)

# output
output = register_module('RootOutput')
output.param('outputFileName', 'MCFittingOutput.root')

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(evtgeninput)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(track_finder_mc_truth)
main.add_module(fitting)
main.add_module('Display')
main.add_module(output)

# Process events
process(main)
print statistics
