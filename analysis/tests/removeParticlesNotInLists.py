#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *


main = create_path()

inputMdst("default", Belle2.FileSystem.findFile('analysis/tests/mdst.root'), path=main)

kaons = ('K-', 'kaonID > 0.05')
pions = ('pi+', 'pionID > 0.05')
photons = ('gamma', '')
fillParticleLists([kaons, pions, photons], path=main)

reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)
reconstructDecay('D0 -> K- pi+ pi0', '1.7 < M < 1.9', 0, path=main)

output = register_module('RootOutput')
output.param('outputFileName', 'removeparticlesnotinlists_full.root')
main.add_module(output)

summaryOfLists(['D0'], path=main)

removeParticlesNotInLists(['D0'], path=main)

matchMCTruth('D0', path=main)
matchMCTruth('pi0', path=main)

# ensures that things in D0 list are ok
ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'removeparticlesnotinlists_D0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'daughter(2, M)', 'mcPDG'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

# pi0 list should also have been fixed
ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'removeparticlesnotinlists_pi0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'mcPDG'])
ntupler.param('particleList', 'pi0')
main.add_module(ntupler)

output = register_module('RootOutput')
output.param('outputFileName', 'removeparticlesnotinlists_reduced.root')
main.add_module(output)

process(main)

print(statistics)

statfull = os.stat('removeparticlesnotinlists_full.root').st_size
statreduced = os.stat('removeparticlesnotinlists_reduced.root').st_size
B2RESULT("original size (kB): " + str(statfull / 1024))
B2RESULT("reduced size (kB):  " + str(statreduced / 1024))
if statfull <= statreduced:
    B2FATAL("Reduced file is not smaller than original")

os.remove('removeparticlesnotinlists_D0ntuple.root')
os.remove('removeparticlesnotinlists_pi0ntuple.root')
os.remove('removeparticlesnotinlists_full.root')
os.remove('removeparticlesnotinlists_reduced.root')
