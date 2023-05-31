#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#######################################################
# 100 pure bkg events are produced
########################################################

"""
<header>
<output>ECLBkgOutput.root</output>
<contact>Priyanka Cheema, pche3675@uni.sydney.edu.au</contact>
</header>
"""

import os
import basf2 as b2
import glob
from simulation import add_simulation
from reconstruction import add_reconstruction

# Create paths
main = b2.create_path()

# Event setting and info
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})
main.add_module(eventinfosetter)

# pGun is used as artifact, otherwise FullSim will complain that no MCParticles Array is available
pGun = b2.register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [2212],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [0.0],
    'thetaGeneration': 'fixed',
    'thetaParams': [0.],
    'phiGeneration': 'fixed',
    'phiParams': [0., ],
    'vertexGeneration': 'fixed',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

# Fixed random seed
b2.set_random_seed(123456)

if 'BELLE2_BACKGROUND_DIR' in os.environ:
    bg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
    b2.B2INFO('Using background samples from ' +
              os.environ['BELLE2_BACKGROUND_DIR'])
else:
    b2.B2FATAL('Environment variable BELLE2_BACKGROUND_DIR is not set.')

add_simulation(main, bkgfiles=bg)
add_reconstruction(main)

# eclDataAnalysis module
ecldataanalysis = b2.register_module('ECLDataAnalysis')
ecldataanalysis.param('rootFileName', '../ECLBkgOutput.root')
ecldataanalysis.param('doTracking', 1)
main.add_module(ecldataanalysis)

main.add_module('Progress')
b2.process(main)
# print(statistics)
