#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to generate particle gun
# MC samples in DST root format
# for ECL charged PID analysis
#
# Author: The Belle II Collaboration
# Contributor: Cate MacQueen
# Contact: cmq.centaurus@gmail.com
#
########################################################

from basf2 import *
import glob
from ROOT import Belle2
from modularAnalysis import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from beamparameters import add_beamparameters

particle_type = sys.argv[1]  # particle mc pdg code
p_min = sys.argv[2]  # input in MeV
p_max = sys.argv[3]  # input in MeV
theta_min = sys.argv[4]  # input in deg
theta_max = sys.argv[5]  # input in deg
file_num = sys.argv[6]  # file numbering scheme

# Create paths
main = create_path()

# Event setting and info
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100], 'runList': [1]})
main.add_module(eventinfosetter)

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


# include beam background
bg = glob.glob('/group/belle2/BGFile/OfficialBKG/15thCampaign/bgoverlay_phase3/bgoverlay*.root')
add_simulation(main, bkgfiles=bg)
add_reconstruction(main)

# output file
output = register_module('RootOutput')
output.param('outputFileName', './MDST_pdg'+str(particle_type)+'_BGx1_'+str(file_num)+'.root')
main.add_module(output)

process(main)
print(statistics)
