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
# basf2 -n 100 EclPartGun.py 11 200 5000 0 180 [OPTIONAL] /PATH/TO/BEAM/BKG/FILES/*.root
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
beam_bkg_path = '/sw/belle2/bkg/*.root'
if sys.argv[6]:
    beam_bkg_path = sys.argv[6]

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
add_simulation(main, bkgfiles=glob.glob(beam_bkg_path))
add_reconstruction(main)

# output file
output = register_module('RootOutput')
output.param('outputFileName', './MDST_pdg'+str(particle_type)+'_BGx1.root')
main.add_module(output)

process(main)
print(statistics)
