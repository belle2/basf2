#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Contributor: Chunhua LI

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import analysis_main
from modularAnalysis import loadReconstructedParticles

logging.log_level = LogLevel.INFO
emptypath = create_path()

filelistSIG = \
    ['/group/belle2/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s00/mixed_e0001r000*BGx1.mdst.root',
     '/group/belle2/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s00/charged_e0001r000*_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistSIG)

histo = register_module('HistoManager')
histo.param('histoFileName', 'histogram.root')
analysis_main.add_module(histo)

# load all final state Particles
# loadReconstructedParticles()

# create charged tracks list
fillParticleList('pi+:HLT', '')

# create gamma list
fillParticleList('gamma:HLT', '')

physicstrigger = register_module('PhysicsTrigger')
analysis_main.add_module(physicstrigger)

physicstriggerDQM = register_module('PhysicsTriggerDQM')
analysis_main.add_module(physicstriggerDQM)

process(analysis_main)

print(statistics)
