#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# Demonstrate parallel processing using full simulation and reconstruction
# chain. The key bit is the nprocess() call at the very end.
#
# Alternatively, you can simply use the -p NPROCESSES argument
# when executing any steering file to enable parallel processing.
##############################################################################

from basf2 import *

from simulation import add_simulation
from reconstruction import add_reconstruction

# suppress messages and warnings during processing:
#set_log_level(LogLevel.ERROR)

particlegun = register_module('ParticleGun')
particlegun.param('pdgCodes', [211, -211, 321, -321])
particlegun.param('nTracks', 5)

eventinfosetter = register_module('EventInfoSetter')

# want to process 100 MC events
eventinfosetter.param('evtNumList', [100])

# Set output filename
output = register_module('RootOutput')
output.param('outputFileName', 'simout.root')

# ============================================================================

main = create_path()
main.add_module(eventinfosetter)
main.add_module(particlegun)

# detecor simulation
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'BKLM',
    'ECL',
    ]
add_simulation(main, components)
# or add_simulation(main) to simulate all detectors

add_reconstruction(main, components)

# output path
main.add_module(output)

# Process events, using 4 parallel processes
nprocess(4)
process(main)

# Print call statistics (not really useful with multiple processes)
# print statistics
