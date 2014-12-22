#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *


main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_100events.root'))


main.add_module(rootinput)
main.add_module(register_module('ParticleLoader'))


selectParticle('K-', 'Kid > 0.05', path=main)
selectParticle('pi+', 'piid > 0.05', path=main)

selectParticle('gamma', '', path=main)
reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)

daughters = ['K-', 'pi+', 'pi0']

reconstructDecay('D0 -> K- pi+ pi0', '1.7 < M < 1.9', 0, path=main)


output = register_module('RootOutput')
output.param('outputFileName', 'discardparticles_full.root')
main.add_module(output)

summaryOfLists(['D0'], path=main)

discardparticles = register_module('DiscardParticles')
discardparticles.param('particleLists', ['D0'])
main.add_module(discardparticles)

matchMCTruth('D0', path=main)
matchMCTruth('pi0', path=main)

# ensures that things in D0 list are ok
ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'discardparticles_D0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'daughter(2, M)', 'mcPDG'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

# pi0 list should also have been fixed
ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'discardparticles_pi0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'mcPDG'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

output = register_module('RootOutput')
output.param('outputFileName', 'discardparticles_reduced.root')
main.add_module(output)

process(main)

print statistics

statfull = os.stat('discardparticles_full.root').st_size
statreduced = os.stat('discardparticles_reduced.root').st_size
B2RESULT("original size (kB): " + str(statfull / 1024))
B2RESULT("reduced size (kB):  " + str(statreduced / 1024))
if statfull <= statreduced:
    B2FATAL("Reduced file is not smaller than original")

os.remove('discardparticles_D0ntuple.root')
os.remove('discardparticles_pi0ntuple.root')
os.remove('discardparticles_full.root')
os.remove('discardparticles_reduced.root')
