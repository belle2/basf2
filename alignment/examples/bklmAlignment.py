#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

main = create_path()

main.add_module('RootInput', inputFileName='fullreco.root', excludeBranchNames=['GF2TracksToMCParticles', 'TrackCandsToGF2Tracks'])

main.add_module('Gearbox')
geometry = register_module('Geometry')
geometry.param({
    "excludedComponents": ["MagneticField"],
    "additionalComponents": ["MagneticField2d"],
})
main.add_module('Geometry',
                excludedComponents=['MagneticField'],
                additionalComponents=['MagneticField2d'])

main.add_module('GBLfit')

main.add_module(
    'PedeSteeringCreator',
    parameters=[
        '6.0.0.0:0.0,-1',
        '5.0.0.0:0.0,-1',
        '4.0.0.0:0.0,-1',
        '3.0.0.0:0.0,-1',
        '2.0.0.0:0.0,-1',
        '1.0.0.0:0.0,-1'])
main.add_module('MillepedeCalibration')

main.add_module('Progress')
main.add_module('GBLdiagnostics')
process(main)

# Print call statistics
print statistics
