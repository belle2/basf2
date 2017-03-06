#!/usr/bin/env python3
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
components = ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC', 'EKLM', 'BKLM']
geometry = register_module('Geometry', components=components)

# simulate only tracking detectors to simulate the whole detector included in
# BelleII.xml:
# geometry.param('Components', ['MagneticField', 'BeamPipe', 'PXD', 'SVD', 'CDC'
#                ])

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

# simulation
g4sim = register_module('FullSim')
g4sim.param('StoreAllSecondaries', True)
g4sim.param('SecondariesEnergyCut', 0.)
g4sim.param('trajectoryStore', 3)
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')

# find MCTracks
track_finder_mc_truth = register_module('TrackFinderMCTruthRecoTracks')

# fitting
fitting = register_module('MillepedeCollector')

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
# main.add_module(evtgeninput)
main.add_module('ParticleGun')
main.add_module(g4sim)
main.add_module('PXDDigitizer')
main.add_module('SVDDigitizer')
main.add_module('PXDClusterizer')
main.add_module('SVDClusterizer')
main.add_module(cdcDigitizer)
main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False, whichGeometry='TGeo')
main.add_module(track_finder_mc_truth)
main.add_module(fitting)
main.add_module('Display', showRecoTracks=True)
main.add_module(output)

# Process events
process(main)
print(statistics)
