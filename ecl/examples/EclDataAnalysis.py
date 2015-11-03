#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use EclDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# The user should provide input and output root filnames
# as first and second argument respectively.
#
# Example steering file - 2014 Belle II Collaboration
########################################################

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = create_path()

# Event setting and info
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

# random number for generation
import random
intseed = random.randint(1, 10000000)

# single particle generator settings
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.1],
    'thetaGeneration': 'uniform',
    'thetaParams': [30., 40.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}

pGun.param(param_pGun)
main.add_module(pGun)

bkgdir = 'bkg/'
# bkg = glob.glob(bkgdir+'*.root')

bkgFiles = [
    bkgdir + 'Coulomb_HER_100us.root',
    bkgdir + 'Coulomb_LER_100us.root',
    bkgdir + 'Coulomb_HER_100usECL.root',
    bkgdir + 'Coulomb_LER_100usECL.root',
    bkgdir + 'RBB_HER_100us.root',
    bkgdir + 'RBB_LER_100us.root',
    bkgdir + 'RBB_HER_100usECL.root',
    bkgdir + 'RBB_LER_100usECL.root',
    bkgdir + 'Touschek_HER_100us.root',
    bkgdir + 'Touschek_LER_100us.root',
    bkgdir + 'Touschek_HER_100usECL.root',
    bkgdir + 'Touschek_LER_100usECL.root',
]

add_simulation(main)
add_reconstruction(main)

display = register_module('Display')
# main.add_module(display)

# eclDataAnalysis module
ecldataanalysis = register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', 'EclDataAnalysis_Example.root')
ecldataanalysis.param('doTracking', 1)
main.add_module(ecldataanalysis)

process(main)
print(statistics)
