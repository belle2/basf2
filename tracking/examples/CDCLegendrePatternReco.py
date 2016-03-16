#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###########################################################################################################################
#
############################################################################################################################

import sys
import os
from optparse import OptionParser

parser = OptionParser()

parser.add_option(
    '-i',
    '--input',
    dest='input',
    default='-999',
    help='location of input file'
)

(options, args) = parser.parse_args()

print(options)

from basf2 import *

set_log_level(LogLevel.INFO)

from trackfindingcdc.cdcdisplay import CDCSVGDisplayModule

# necessary modumes
eventinfosetter = register_module('EventInfoSetter')
beamparameter = register_module('BeamParameters')
# generate one event
eventinfosetter.param('expList', [0])
eventinfosetter.param('runList', [1])
eventinfosetter.param('evtNumList', [10])
eventinfoprinter = register_module('EventInfoPrinter')

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

input = register_module('RootInput')
input.param('inputFileName', options.input)

# simulate only tracking detectors to simulate the whole detector included in
# BelleII.xml, comment the next line
# geometry.param('components', ['MagneticField',
# 'BeamPipe',
# 'PXD',
# 'SVD',
# 'CDC'])


# ---------------------------------------------------------------


particlegun = register_module('ParticleGun')
# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks But vary the number of tracks
                     # according to Poisson distribution Generate pi+, pi-, e+
                     # and e- with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV a normal
                     # distributed phi angle, center of 180 degree and a width
                     # of 30 degree Generate theta angles uniform in cos theta
                     # between 17 and 150 degree normal distributed vertex
                     # generation around the origin with a sigma of 2cm in the
                     # xy plane and no deviation in z all tracks sharing the
                     # same vertex per event
    'nTracks': 1,
    'varyNTracks': False,
    'pdgCodes': [-211],  # 211-pi, 321-K, 2212-p
    'momentumGeneration': 'fixed',  # 'uniform',
    'momentumParams': [0.6],  # , 0.7],
    'phiGeneration': 'fixed',
    'phiParams': [135],  # , 281],
    'thetaGeneration': 'uniform',
    'thetaParams': [100, 110],
    'vertexGeneration': 'normal',
    'xVertexParams': [-5., 0.005],
    'yVertexParams': [-5., 0.005],
    'zVertexParams': [0, 0],
    'independentVertices': False,
})

# EvtGen to provide generic BB events
evtgeninput = register_module('EvtGenInput')

# simulation
g4sim = register_module('FullSim')
# this is need for the MCTrackFinder to work correctly
g4sim.param('StoreAllSecondaries', False)
g4sim.param('SecondariesEnergyCut', 0.1)

# make the simulation less noisy
g4sim.logging.log_level = LogLevel.ERROR

# digitizer
cdcDigitizer = register_module('CDCDigitizer')
cdcDigitizer.param({
    'AddInWirePropagationDelay': True,
    'AddTimeOfFlight': True,
})

wirehittopologypreparer = register_module('WireHitTopologyPreparer')

# TrackFinderCDCLegendre
cdctracking = register_module('TrackFinderCDCLegendreTracking')
trackCandidateConverter = register_module('GenfitTrackCandidatesCreator')

# Display module
display_module = CDCSVGDisplayModule()
display_module.draw_gftrackcand_trajectories = True
display_module.draw_gftrackcands = True
display_module.output_folder = "tmp/"
display_module.use_time_in_filename = False
display_module.filename_prefix = "LegendreFinder"
display_module.interactive = False


printcollections = register_module('PrintCollections')

# create paths
main = create_path()

# add modules to paths
if options.input == '-999':
    main.add_module(eventinfosetter)
    main.add_module(eventinfoprinter)
    main.add_module(gearbox)
    main.add_module(geometry)
    main.add_module(beamparameter)
    # main.add_module(particlegun1)
    main.add_module(evtgeninput)
    main.add_module(g4sim)
    main.add_module(cdcDigitizer)

else:
    main.add_module(input)
    main.add_module(gearbox)
    main.add_module(geometry)


main.add_module(wirehittopologypreparer)

main.add_module(cdctracking)
main.add_module(trackCandidateConverter)

main.add_module(display_module)

# main.add_module(printcollections)

# Process events
process(main)
print(statistics)
