#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

from ROOT import Belle2

main = create_path()

main.add_module('RootInput', inputFileName=sys.argv[1])
main.add_module('HistoManager', histoFileName=sys.argv[2])
main.add_module('Gearbox')
geometry = register_module('Geometry', excludedComponents=['MagneticField'],
                           additionalComponents=["MagneticField2d"])
main.add_module(geometry)

main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False,
                noiseCoulomb=False, noiseBrems=False)
main.add_module('MillepedeCollector', components='EKLMAlignment',
                useGblTree=False)

main.add_module('Progress')
process(main)

# Print call statistics
print(statistics)
