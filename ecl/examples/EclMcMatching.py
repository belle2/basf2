#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

set_log_level(LogLevel.ERROR)
set_random_seed(75628607)

# Register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfoprinter = register_module('EventInfoPrinter')

# Create geometry
# Geometry parameter loader
gearbox = register_module('Gearbox')

# Geometry builder
geometry = register_module('Geometry')

# Simulation
g4sim = register_module('FullSim')

# one event
eventinfosetter.param({'evtNumList': [3], 'runList': [1]})

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
rofdir1 = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/RBB_HER/'
bgmixer1 = register_module('MixBkg')
bgmixer1.param('BackgroundFiles', [rofdir1 + 'ECLROFnoMC_RBB_HER_1ms_*'])
bgmixer1.set_log_level(LogLevel.INFO)

rofdir2 = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/RBB_LER/'
bgmixer2 = register_module('MixBkg')
bgmixer2.param('BackgroundFiles', [rofdir2 + 'ECLROFnoMC_RBB_LER_1ms_*'])
bgmixer2.set_log_level(LogLevel.INFO)

rofdir3 = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/Coulomb_HER/'
bgmixer3 = register_module('MixBkg')
bgmixer3.param('BackgroundFiles', [rofdir3 + 'ECLROFnoMC_Coulomb_HER_1ms_*'])
bgmixer3.set_log_level(LogLevel.INFO)

rofdir4 = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/Coulomb_LER/'
bgmixer4 = register_module('MixBkg')
bgmixer4.param('BackgroundFiles', [rofdir4 + 'ECLROFnoMC_Coulomb_LER_1ms_*'])
bgmixer4.set_log_level(LogLevel.INFO)

rofdir5 = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/Touschek_HER/'
bgmixer5 = register_module('MixBkg')
bgmixer5.param('BackgroundFiles', [rofdir5 + 'ECLROFnoMC_Touschek_HER_1ms_*'])
bgmixer5.set_log_level(LogLevel.INFO)

rofdir6 = '/gpfs/fs02/belle2/users/BGFile/MCprod_2013Summer/Touschek_LER/'
bgmixer6 = register_module('MixBkg')
bgmixer6.param('BackgroundFiles', [rofdir6 + 'ECLROFnoMC_Touschek_LER_1ms_*'])
bgmixer6.set_log_level(LogLevel.INFO)


eclDigi = register_module('ECLDigitizer')
eclRecShower = register_module('ECLReconstructor')
makeGamma = register_module('ECLGammaReconstructor')
makePi0 = register_module('ECLPi0Reconstructor')
makeMatch = register_module('ECLMCMatching')

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(pGun)
main.add_module(g4sim)
main.add_module(bgmixer1)
main.add_module(bgmixer2)
main.add_module(bgmixer3)
main.add_module(bgmixer4)
main.add_module(bgmixer5)
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
