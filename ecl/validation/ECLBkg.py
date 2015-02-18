#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# BASF2 (Belle Analysis Framework 2)                                     *
# Copyright(C) 2011 - Belle II Collaboration                             *
#
# Author: The Belle II Collaboration                                     *
# Contributors: Benjamin Oberhof
# This steering file generates an ntuple for validation
# purposes
########################################################

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction

set_log_level(LogLevel.ERROR)

# Register necessary modules
eventinfosetter = register_module('EventInfoSetter')
# Set the number of events to be processed (1000 events)
eventinfosetter.param({'evtNumList': [1000], 'runList': [1]})

eventinfoprinter = register_module('EventInfoPrinter')

# Fixed random seed
set_random_seed(123456)

pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [22],
    'nTracks': 1,
    'momentumGeneration': 'fixed',
    'momentumParams': [1],
    'thetaGeneration': 'uniform',
    'thetaParams': [0., 180.],
    'phiGeneration': 'uniform',
    'phiParams': [0, 360],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0., 0.],
    'yVertexParams': [0., 0.],
    'zVertexParams': [0., 0.],
    }
pGun.param(param_pGun)

# simpleoutput = register_module('RootOutput')
# simpleoutput.param('outputFileName', '../ECLPionOutput.root')
eclanalysis = register_module('ECLDataAnalysis')
eclanalysis.param('rootFileName', '../ECLBkgOutput.root')
# eclanalysis.param('doTracking', 1)

# Add Bkg in Simulation
bkgdir = '$BELLE2_BACKGROUND_DIR/'
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

# Create paths
main = create_path()
main.add_module(eventinfosetter)
main.add_module(eventinfoprinter)
# main.add_module(pGun)
add_simulation(main, bkgfiles=bkgFiles)
add_reconstruction(main)
main.add_module(eclanalysis)
# main.add_module(simpleoutput)

process(main)
print statistics
