#!/usr/bin/env python
# -*- coding: utf-8 -*-

######### Imports #########
import optparse

from basf2 import *
from ROOT import Belle2

import simulation

import matplotlib.pyplot as plt

########## Register modules  ##########
eventInfoSetterModule = register_module('EventInfoSetter')
eventInfoSetterModule.param({'evtNumList': [10], 'runList': [1],
                            'expList': [1]})

progressModule = register_module('Progress')

particleGunModule = register_module('ParticleGun')
particleGunModule.param({
    'pdgCodes': [13, -13],
    'nTracks': 10,
    'varyNTracks': False,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.6, 1.4],
    'thetaGeneration': 'uniform',
    'thetaParams': [17., 150.],
    })

rootOutputModule = register_module('RootOutput')
rootOutputModule.param({'outputFileName': 'mc_gun.root'})

########## Create paths and add modules  ##########
main = create_path()

main.add_module(eventInfoSetterModule)
main.add_module(progressModule)

main.add_module(particleGunModule)

components = ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD',
              'CDC']

simulation.add_simulation(main, components=components)

main.add_module(rootOutputModule)

process(main)

# Print call statistics
print statistics
