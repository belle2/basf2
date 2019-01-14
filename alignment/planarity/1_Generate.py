#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *
import simulation
import reconstruction
import svd
import pxd

from ROOT import Belle2
from ROOT import TVector3
import ROOT

# register necessary modules
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10000])

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('components', ['PXD', 'SVD'])
# geometry.param('useDB', False)

particlegun = register_module('ParticleGun')
particlegun.param('nTracks', 1)
particlegun.param('pdgCodes', [13, -13])
particlegun.param('varyNTracks', False)

particlegun.param('momentumGeneration', 'fixed')
particlegun.param('momentumParams', [10])

particlegun.param('thetaGeneration', 'uniform')
particlegun.param('thetaParams', [70, 110])

particlegun.param('phiGeneration', 'uniform')
particlegun.param('phiParams', [250, 290])

particlegun.param('xVertexGeneration', 'uniform')
particlegun.param('xVertexParams', [-1, 7])
particlegun.param('yVertexGeneration', 'fixed')
particlegun.param('yVertexParams', [25])
particlegun.param('zVertexGeneration', 'uniform')
particlegun.param('zVertexParams', [-5, 10])
particlegun.param('independentVertices', False)
print_params(particlegun)

# create paths
main = create_path()

# add modules to paths
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(particlegun)
simulation.add_simulation(main, components=['SVD', 'PXD'], usePXDDataReduction=False)

main.add_module('RootOutput')

main.add_module('ProgressBar')
# main.add_module('Progress')

process(main)
print(statistics)
