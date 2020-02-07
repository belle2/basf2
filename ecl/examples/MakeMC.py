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

import sys
import glob
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction

particle_type = sys.argv[1]  # particle mc pdg code
p_min = sys.argv[2]  # input in MeV
p_max = sys.argv[3]  # input in MeV
theta_min = sys.argv[4]  # input in deg
theta_max = sys.argv[5]  # input in deg
file_num = sys.argv[6]  # file numbering scheme

# Create paths
main = b2.create_path()

# Event setting and info
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [100],
                       'runList': [1],
                       'expList': [0]})

main.add_module(eventinfosetter)

b2.set_random_seed(123456)
b2.set_log_level(b2.LogLevel.ERROR)

# generator settings
pGun = b2.register_module('ParticleGun')
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
output = b2.register_module('RootOutput')
output.param('outputFileName',
             './MDST_pdg{}_BGx1_{}.root'.format(particle_type, file_num))

main.add_module(output)

b2.process(main)
print(b2.statistics)
