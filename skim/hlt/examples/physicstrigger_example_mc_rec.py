#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################################
#
# This file shows how to execute high level trigger (hlt) module
# The data flow isGenerator-->Simulation-->Level3 trigger-->Reconstruction->Physics trigger
#
# For reconstruction module, events are reconstructed with information form all
# detectors except PXD
#
#
# Contributor: Chunhua LI
#
##################################################################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from modularAnalysis import generateY4S
from HLTTrigger import add_HLT_Y4S
from beamparameters import add_beamparameters
from modularAnalysis import generateY4S
from ROOT import Belle2

logging.log_level = LogLevel.INFO
emptypath = create_path()

filelistSIG = \
    [
        '../../../../release-head-head/MC/Analysis_L1emulator/MC/tauToegamma/BGx0-noVXD/rootFiles/*root'
    ]

inputMdstList(filelistSIG)

# create charged tracks list
fillParticleList('pi+:HLT', '')

# create gamma list
fillParticleList('gamma:HLT', '')

# load the PhysicsTrigger Module
add_HLT_Y4S(analysis_main)


# outputs
branches = ['HLTTags', 'PhysicsTriggerInformations']

output = register_module('RootOutput')
output.param('outputFileName', 'output_physicstrigger.root')
output.param('branchNames', branches)
analysis_main.add_module(output)

process(analysis_main)

print(statistics)
