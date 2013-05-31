#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

set_log_level(LogLevel.ERROR)
set_random_seed(75628607)

# Register necessary modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')

# Create geometry
# Geometry parameter loader
gearbox = register_module('Gearbox')

# Geometry builder
geometry = register_module('Geometry')

# Simulation
g4sim = register_module('FullSim')

# one event
evtmetagen.param({'evtNumList': [3], 'runList': [1]})

import random
intseed = random.randint(1, 10000000)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22, 111],
    'nTracks': 6,
    'momentumGeneration': 'uniform',
    'momentumParams': [1., 2.],
    'thetaGeneration': 'uniform',
    'thetaParams': [50., 130.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)

# Mix some background to simulation data
rofdir = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/RBB_LER/'
bgmixer6 = register_module('MixBkg')
# bgmixer1.param('BackgroundFiles',[rofdir+'PXDROF_RBB_LER_1ms_0x.root'])
# bgmixer2.param('BackgroundFiles',[rofdir+'SVDROF_RBB_LER_1ms_0x.root'])
# bgmixer3.param('BackgroundFiles',[rofdir+'CDCROF_RBB_LER_1ms_0x.root'])
# bgmixer4.param('BackgroundFiles',[rofdir+'TOPROF_RBB_LER_1ms_0x.root'])
# bgmixer5.param('BackgroundFiles',[rofdir+'ARICHROF_RBB_LER_1ms_0x.root'])
bgmixer6.param('BackgroundFiles', [rofdir + 'ECLROF_RBB_LER_1ms_0x.root'])
# bgmixer7.param('BackgroundFiles',[rofdir+'EKLMROF_RBB_LER_1ms_0x.root'])
# bgmixer8.param('BackgroundFiles',[rofdir+'BKLMROF_RBB_LER_1ms_0x.root'])
# bgmixer9.param('BackgroundFiles',[rofdir+'ECLsimROF_RBB_LER_1ms_0x.root'])
bgmixer6.set_log_level(LogLevel.INFO)

eclDigi = register_module('ECLDigitizer')
eclRecShower = register_module('ECLReconstructor')
makeGamma = register_module('ECLGammaReconstructor')
makePi0 = register_module('ECLPi0Reconstructor')
makeMatch = register_module('ECLMCMatching')

# Create paths
main = create_path()
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(bgmixer6)
main.add_module(eclDigi)
main.add_module(eclRecShower)
main.add_module(makeGamma)
main.add_module(makePi0)
main.add_module(makeMatch)
simpleoutput = register_module('RootOutput')
simpleoutput.param('outputFileName', 'Output.root')
main.add_module(simpleoutput)

process(main)
print statistics
