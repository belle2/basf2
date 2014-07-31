#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
hltInput = fw.register_module('HLTInput')
evtMetaGen = fw.register_module('EventInfoSetter')
gearBox = fw.register_module('Gearbox')
particleGun = fw.register_module('ParticleGun')
geoBuilder = fw.register_module('Geometry')
fullSim = fw.register_module('FullSim')
perfTest = fw.register_module('PerfTest')
hltOutput = fw.register_module('HLTOutput')
simpleOutput = fw.register_module('SimpleOutput')
pRootOutput = fw.register_module('pRootOutput')
seqRootOutput = fw.register_module('SeqRootOutput')
perfTest = fw.register_module('PerfTest')

# Parameter settings
evtMetaGen.param({'EvtNumList': [1], 'RunList': [1]})
particleGun.param('randomSeed', 1028307)
particleGun.param('pdgCodes', [-11, 11])
particleGun.param('nTracks', 10)
particleGun.param('varyNTracks', False)
particleGun.param('momentumGeneration', 'uniform')
particleGun.param('momentumParams', [0.05, 3])
particleGun.param('thetaGeneration', 'uniform')
particleGun.param('thetaParams', [17, 150])
particleGun.param('phiGeneration', 'uniform')
particleGun.param('phiParams', [17, 150])
particleGun.param('vertexGeneration', 'normal')
particleGun.param('xVertexParams', [0, 0.7])
particleGun.param('xVertexParams', [0, 0.7])
particleGun.param('yVertexParams', [0, 0.7])
particleGun.param('zVertexParams', [0, 1.0])
particleGun.param('independentVertices', False)

hltInput.param('nodeType', 'WN')
hltOutput.param('nodeType', 'WN')
perfTest.param('outputFileName', 'performance.single.txt')

main = fw.create_path()

# main.add_module(hltInput)
main.add_module(perfTest)
main.add_module(evtMetaGen)
main.add_module(particleGun)
main.add_module(gearBox)
main.add_module(geoBuilder)
main.add_module(fullSim)
# main.add_module (hltOutput)
main.add_module(simpleOutput)
# main.add_module (pRootOutput)
# main.add_module (seqRootOutput)
main.add_module(perfTest)

fw.process(main)
