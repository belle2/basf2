#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################################
#
# This file shows how to configure the physics trigger by users. The input
# data in this example is mdst files with reconstruction.
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
from HLTTrigger import add_HLT_Y4S
from HLTTrigger import PTrigger
from HLTTrigger import FillHLTTag
from beamparameters import add_beamparameters
from ROOT import Belle2


logging.log_level = LogLevel.INFO
emptypath = create_path()

inputMdstList(inputfile)

# create charged tracks list
fillParticleList('pi+:HLT', '')

# create gamma list
fillParticleList('gamma:HLT', '')

# load the PhysicsTrigger Module
# add_HLT_Y4S(analysis_main)

# configure the users's trigger
# This function adds the physics trigger module to a path
# parameter one: the name of the trigger
# parameter two: the selection criteria of physics trigger
# parameter three: the path
# parameter four: the index of trigger items, start from 1, the catregory
# that the trigger result will be stored, in HLTTag, 1: Hadron, 2: Tautau,
# 3: LowMulti, 4: Bhabha, 5: Dimu, 6: GG, 7: other

# some examples
PTrigger('Hadron', 'NtHLT>=3 and VisibleEnergyHLT>2.5', analysis_main, 1)

PTrigger('BhabhaAccept2', 'NtHLT>=1 and PT1CMSHLT>=3.0 and AngGTHLT>2.618 and ENeutralHLT>2.0 ', analysis_main, 4)

PTrigger('2Trk', 'NtHLT>=2 and NtHLT<=5 and PT1CMSHLT<5.0 and PT12CMSHLT<9.0 ', analysis_main, 3)

# you may add more triggers......

# Fill the trigger result in HLTTag
FillHLTTag(analysis_main, False)

# outputs
branches = ['HLTTags', 'PhysicsTriggerInformations']

output = register_module('RootOutput')
output.param('outputFileName', 'output_physicstrigger.root')
output.param('branchNames', branches)
analysis_main.add_module(output)

process(analysis_main)

print(statistics)
