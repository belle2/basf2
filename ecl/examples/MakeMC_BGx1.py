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
# import random
from basf2 import *
import glob
from ROOT import Belle2
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters

particle_type = sys.argv[1]
p_min = sys.argv[2]  # input in MeV
p_max = sys.argv[3]  # input in MeV
theta_min = sys.argv[4]  # input in deg
theta_max = sys.argv[5]  # input in deg

# Create paths
main = create_path()

# Event setting and info
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10000], 'runList': [1]})
main.add_module(eventinfosetter)

# random number for generation
# import random
# intseed = random.randint(1, 1)

set_random_seed(123456)
set_log_level(LogLevel.ERROR)

# generator settings
pGun = register_module('ParticleGun')
param_pGun = {
    'pdgCodes': [int(particle_type)],
    'nTracks': 1,
    'momentumGeneration': 'uniform',
    'momentumParams': [float(p_min) / 1000, float(p_max) / 1000],
    'thetaGeneration': 'uniform',
    'thetaParams': [float(theta_min), float(theta_max)],
    'phiGeneration': 'uniform',
    'phiParams': [0., 360.],
    'vertexGeneration': 'uniform',
    'xVertexParams': [0.0, 0.0],
    'yVertexParams': [0.0, 0.0],
    'zVertexParams': [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

bgfolder = '/sw/belle2/bkg'
bg = glob.glob(bgfolder + '/*.root')
add_simulation(main, bkgfiles=bg, bkgscale=1)

# bkg = glob.glob(os.environ['BELLE2_BACKGROUND_DIR']+'*.root')

# add_simulation(main)
add_reconstruction(main)

output = register_module('RootOutput')
output.param('outputFileName', '/hsm/belle2/bdata/users/cguenthe/LeptonIDWork/MDST/BGx1/pdg' + str(particle_type) +
             '_mom' + str(p_min) + 'to' + str(p_max) + 'MeV_theta' + str(theta_min) + 'to' + str(theta_max) + 'deg.root')
main.add_module(output)

process(main)
print(statistics)
