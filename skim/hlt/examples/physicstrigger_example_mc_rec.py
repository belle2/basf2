#!/usr/bin/env python
# -*- coding: utf-8 -*-

################################################################################################
#
# This file shows how to execute high level trigger (hlt) module
# The data flow isGenerator-->Simulation-->Level3 trigger-->Reconstruction->Physics trigger
#
# For reconstruction module, events are reconstructed with information form all
# detectors except PXD
#
# For PhyscisTriger module, to be flexible, the user custom option is open.
# If you want to select the events by using yourself selection criteria,
# just set UserCustomOpen to a value larger than zero as commented sentences below,
# and provide your selection criteria to UserCustomCut.
#
# physicstrigger.param('UserCustomOpen',1); (default: 0)
# physicstrigger.param('UserCustomCut','2<=nTracksHLT<=10'); (default: empty)
#
# Please pay attention that only variables defined in VariableManager/PhysicsTriggerVariables{h,cc}
# could be used in your selection criteria.
# If you want to use other variables defined by yourself, please define it in PhysicsTriggerVariables.cc firstly.
# Once user custom is open, the official selection criteria will be closed automaticly.
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
from modularAnalysis import loadReconstructedParticles
from modularAnalysis import generateY4S

logging.log_level = LogLevel.INFO
emptypath = create_path()

filelistSIG = \
    ['/group/belle2/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s00/mixed_e0001r000*BGx1.mdst.root'
     ,
     '/group/belle2/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s00/charged_e0001r000*_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistSIG)

# load all final state Particles
loadReconstructedParticles()

# create charged tracks list
fillParticleList('pi+:HLT', '')

# create gamma list
fillParticleList('gamma:HLT', '')

physicstrigger = register_module('PhysicsTrigger')
# physicstrigger.param('UserCustomOpen',1);
# physicstrigger.param('UserCustomCut','2<=nTracksHLT<=10');
analysis_main.add_module(physicstrigger)

# uncomment if you want to turn on the physics-trigger
# emptypath = create_path()
# physicstrigger.if_false(emptypath)

# outputs
branches = ['HLTTags', 'L3Tags', 'L3Tracks', 'L3Clusters',
            'PhysicsTriggerInformations']

output = register_module('RootOutput')
output.param('outputFileName', 'output_physicstrigger.root')
output.param('branchNames', branches)
analysis_main.add_module(output)

process(analysis_main)

print statistics
