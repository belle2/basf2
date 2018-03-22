#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from HLTTrigger import add_HLT_Y4S
from ROOT import Belle2

logging.log_level = LogLevel.INFO
emptypath = create_path()

filelistSIG = \
    [
        'inputfile'
    ]

inputMdstList(filelistSIG)

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
