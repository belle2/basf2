#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import basf2
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

from ROOT import Belle2

basf2.conditions.append_testing_payloads('localdb/database.txt')

main = basf2.create_path()

main.add_module('RootInput', inputFileName=sys.argv[1])
main.add_module('HistoManager', histoFileName=sys.argv[2])
main.add_module('Gearbox')
main.add_module('Geometry')

main.add_module('SetupGenfitExtrapolation',
                noiseBetheBloch=False,
                noiseCoulomb=False,
                noiseBrems=False)

main.add_module('DAFRecoFitter', resortHits=True)

main.add_module('MillepedeCollector',
                components=['BKLMAlignment', 'EKLMAlignment',
                            'EKLMSegmentAlignment'],
                useGblTree=True,
                minPValue=1e-5)

main.add_module('Progress')
basf2.process(main)

# Print call statistics.
print(basf2.statistics)
