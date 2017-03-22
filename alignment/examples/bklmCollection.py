#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

from ROOT import Belle2

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

main.add_module('SetupGenfitExtrapolation', whichGeometry='TGeo', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
main.add_module('MillepedeCollector')

main.add_module('Progress')
# main.add_module('GBLdiagnostics')
main.add_module('RootOutput')
process(main)

# Print call statistics
print(statistics)
