#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use EclDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# Author: The Belle II Collaboration
# Contributors: Benjamin Oberhof
#
########################################################

# import os
# import glob
import random
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters

# Create paths
main = b2.create_path()

# Event setting and info
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100],
                       'runList': [1],
                       'expList': [0]})

main.add_module(eventinfosetter)

# random number for generation
intseed = random.randint(1, 1)

# generator settings
pGun = b2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [13],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.5],
    'thetaGeneration': 'uniform',
    'thetaParams': [0., 30.],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

# bkg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR']+'*.root')

add_simulation(main)
add_reconstruction(main)

# display = b2.register_module('Display')
# main.add_module(display)

# eclDataAnalysis module
ecldataanalysis = b2.register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', 'EclDataAnalysis_Test.root')
ecldataanalysis.param('doTracking', 1)
ecldataanalysis.param('doPureCsIStudy', 0)
main.add_module(ecldataanalysis)

b2.process(main)
print(b2.statistics)
