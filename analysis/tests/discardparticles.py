#!/usr/bin/env python
# -*- coding: utf-8 -*-


import os
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *


main = create_path()

rootinput = register_module('RootInput')
rootinput.param('inputFileName', Belle2.FileSystem.findFile('analysis/tests/mdst_r10142.root'))


main.add_module(rootinput)
main.add_module(register_module('ParticleLoader'))


selectParticle('K-', 'Kid > 0.05', path=main)
selectParticle('pi+', 'piid > 0.05', path=main)

selectParticle('gamma', '', path=main)
reconstructDecay('pi0 -> gamma gamma', '0.11 < M < 0.15', 0, path=main)

daughters = ['K-', 'pi+', 'pi0']

reconstructDecay('D0 -> K- pi+ pi0', '1.7 < M < 1.9', 0, path=main)


#main.add_module(register_module('PrintCollections'))

output = register_module('RootOutput')
output.param('outputFileName', '1.root')
main.add_module(output)

summaryOfLists(['D0'], path=main)

discardparticles = register_module('DiscardParticles')
discardparticles.param('particleLists', ['D0'])
main.add_module(discardparticles)

matchMCTruth('D0', path=main)
matchMCTruth('pi0', path=main)

ntupler = register_module('VariablesToNtuple')
ntupler.param('fileName', 'test_D0ntuple.root')
ntupler.param('variables', ['M', 'daughter(0, M)', 'mcPDG'])
ntupler.param('particleList', 'D0')
main.add_module(ntupler)

#main.add_module(register_module('PrintCollections'))

output = register_module('RootOutput')
output.param('outputFileName', '2.root')
main.add_module(output)

process(main)

print statistics
