#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ####### Imports #######
import basf2 as b2

import simulation

# ######## Register modules  ########
eventInfoSetterModule = b2.register_module('EventInfoSetter')
eventInfoSetterModule.param({'evtNumList': [10], 'runList': [1],
                             'expList': [0]})

progressModule = b2.register_module('Progress')

particleGunModule = b2.register_module('ParticleGun')
particleGunModule.param({
    'pdgCodes': [13, -13],
    'nTracks': 10,
    'varyNTracks': False,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.6, 1.4],
    'thetaGeneration': 'uniform',
    'thetaParams': [17., 150.],
})

rootOutputModule = b2.register_module('RootOutput')
rootOutputModule.param({'outputFileName': 'mc_gun.root'})

# ######## Create paths and add modules  ########
main = b2.create_path()

main.add_module(eventInfoSetterModule)
main.add_module(progressModule)

main.add_module(particleGunModule)

components = ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD',
              'CDC']

simulation.add_simulation(main, components=components)

main.add_module(rootOutputModule)

b2.process(main)

# Print call statistics
print(b2.statistics)
