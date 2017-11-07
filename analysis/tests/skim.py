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

# in DataStore with ID 'skim': {{{
skimpath = create_path()
summaryOfLists(['D0'], path=skimpath)

removeParticlesNotInLists(['D0'], path=skimpath)

matchMCTruth('D0', path=skimpath)
matchMCTruth('pi0', path=skimpath)

# ensures that things in D0 list are ok
ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'skimtest_D0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'daughter(2, M)', 'mcPDG'])
ntupler.param('particleList', 'D0')
skimpath.add_module(ntupler)

# pi0 list should also have been fixed
ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'skimtest_pi0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'mcPDG'])
ntupler.param('particleList', 'pi0')
skimpath.add_module(ntupler)

output = register_module('RootOutput')
output.param('outputFileName', 'skimtest_reduced.root')
skimpath.add_module(output)

# }}}
# main.add_independent_path(skimpath, 'skim')
main.add_independent_path(skimpath)  # with auto-generated name

# Important difference to removeparticlesnotinlists test:
# we save the skimmed  file _before_ the full output
output = register_module('RootOutput')
output.param('outputFileName', 'skimtest_full.root')
main.add_module(output)


process(main)

print(statistics)

statfull = os.stat('skimtest_full.root').st_size
statreduced = os.stat('skimtest_reduced.root').st_size
B2RESULT("full size (kB): " + str(statfull / 1024))
B2RESULT("reduced size (kB):  " + str(statreduced / 1024))
if statfull <= statreduced:
    B2FATAL("Reduced file is not smaller than original")

os.remove('skimtest_D0ntuple.root')
os.remove('skimtest_pi0ntuple.root')
os.remove('skimtest_full.root')
os.remove('skimtest_reduced.root')
