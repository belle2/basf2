#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

set_log_level(LogLevel.ERROR)

# Register necessary modules
evtmetagen = register_module('EvtMetaGen')
evtmetainfo = register_module('EvtMetaInfo')

# Create geometry
# Geometry parameter loader
gearbox = register_module('Gearbox')

# Geometry builder
geometry = register_module('Geometry')

# Simulation
pGun = register_module('PGunInput')
g4sim = register_module('FullSim')

simpleoutput = register_module('SimpleOutput')

# one event
evtmetagen.param('ExpList', [0])
evtmetagen.param('RunList', [1])
evtmetagen.param('EvtNumList', [10])

import random
intseed = random.randint(1, 10000000)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 5,
    'momentumGeneration': 'uniform',
    'momentumParams': [1., 1.],
    'thetaGeneration': 'fixed',
    'thetaParams': [50., 130.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
    }

pGun.param(param_pGun)

eclHit = register_module('ECLHit')
eclDigi = register_module('ECLDigi')
eclRecShower = register_module('ECLRecShower')
makeGamma = register_module('ECLRecGamma')
makePi0 = register_module('ECLRecPi0')
param_Gamma = {
    'gammaEnergyCut': 0.02,
    'gammaE9o25Cut': 0.75,
    'gammaWidthCut': 6.0,
    'gammaNhitsCut': 0,
    }

makeGamma.param(param_Gamma)
simpleoutput.param('outputFileName', 'output1.root')

# Create paths
main = create_path()
main.add_module(evtmetagen)
main.add_module(evtmetainfo)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(cdcDigitizer)
main.add_module(mctrackfinder)
main.add_module(trackfitter)
main.add_module(trackfitchecker)
main.add_module(ext)
main.add_module(eclHit)
main.add_module(eclDigi)
main.add_module(eclRecShower)
main.add_module(makeGamma)
main.add_module(makePi0)
main.add_module(simpleoutput)

process(main)
print statistics
