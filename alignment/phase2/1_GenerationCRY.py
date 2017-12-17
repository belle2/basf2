#!/usr/bin/env python
# -*- coding: utf-8 -*-

# *****************************************************************************

# title           : 1_Generation.py
# description     : Generation & simulation cosmic events (CRY generator)
# author          : Jakub Kandra (jakub.kandra@karlov.mff.cuni.cz)
# date            : 7. 11. 2017

# *****************************************************************************

from basf2 import *
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_cosmics_reconstruction
import tracking
from ROOT import Belle2
from svd import add_svd_reconstruction
from pxd import add_pxd_reconstruction
from EventSelector import EventSelector
import sys
import math
import os

outname = "generation.root"

import sys

if len(sys.argv) == 2:
    print(sys.argv)[1]
    outname = (sys.argv)[1]

main = create_path()

# Event Info
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [1000])  # we want to process 1000 events
eventinfosetter.param('runList', [0])  # from run number 1
eventinfosetter.param('expList', [1002])  # and experiment number 1002 (Phase2)
main.add_module(eventinfosetter)

# Progressbar
progress = register_module('Progress')
main.add_module(progress)

# Register the CRY module
cry = register_module('CRYInput')

# cosmic data input
cry.param('CosmicDataDir', Belle2.FileSystem.findFile('data/generators/modules/cryinput/'))

# user input file
cry.param('SetupFile', 'cry.setup')

# acceptance half-lengths - at least one particle has to enter that box to use that event
cry.param('acceptLength', 0.45)
cry.param('acceptWidth', 0.45)
cry.param('acceptHeight', 0.45)
cry.param('maxTrials', 1000000)

# keep half-lengths - all particles that do not enter the box are removed (keep box >= accept box)
cry.param('keepLength', 0.45)
cry.param('keepWidth', 0.45)
cry.param('keepHeight', 0.45)

# minimal kinetic energy - all particles below that energy are ignored
cry.param('kineticEnergyThreshold', 0.01)

# Reset the top volume: must be larger than the generated surface and higher than the detector
# It is the users responsibility to ensure a full angular coverage

main.add_module("Gearbox", fileName='/geometry/Beast2_phase2.xml', override=[
    ("/Global/length", "300.0", "m"),
    ("/Global/width", "300.0", "m"),
    ("/Global/height", "300.0", "m"), ])

# Register the geometry module
geometry = register_module('Geometry')
components = [
    'BeamPipe',
    'MagneticField',
    'PXD',
    'SVD',
    'CDC',
    'EKLM',
    'BKLM']

"""
'TOP',
'ARICH',
'ECL'
"""

geometry.param('components', components)
main.add_module(geometry)

main.add_module(cry)

add_simulation(main, components=components, usePXDDataReduction=False)

store = create_path()
EventSelector = EventSelector()
EventSelector.if_true(store, AfterConditionPath.CONTINUE)
main.add_module(EventSelector)

# output
output = register_module('RootOutput')
output.param('outputFileName', outname)
store.add_module(output)

# Process the events
process(main)

# Process the events
print(statistics)
