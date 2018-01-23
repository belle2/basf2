#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This demonstrates how to skim the
# following  decay chain:
#
# B0 -> rho gamma
#       |
#       +-> pi+ pi-
#
#
# Note: This example uses the MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
# It also works on the Grid.
# Contributors: C-L Hsu (Oct 2014)
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

set_log_level(LogLevel.INFO)

# Run this either over signal MC or background MC (K*gamma)
# Add 10 signal MC files (each containing 1000 generated events)
filelistBKG = \
    ['/group/belle2/MC/signal/B2Kstargamma/mcprod1405/BGx0/mc35_B2Kstargamma_BGx0_s00/B2Kstargamma_e0001r0001_s00_BGx0.mdst.root'
     ]
filelistSIG = \
    ['/group/belle2/MC/signal/B2Rhogamma/mcprod1405/BGx1/mc35_B2Rhogamma_BGx1_s00/B2Rhogamma_e0001r0001_s00_BGx1.mdst.root'
     ]

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma MC
inputMdstList(filelistBKG)

# use standard final state particle lists
#
# creates "gamma:highE" ParticleList
# contains all photon candidates with E>1.5 GeV
loadStdPhotonE15()

# creates "pi+:loose" ParticleList (and c.c.)
loadStdCharged()
# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
reconstructDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.0')

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
reconstructDecay('B0 -> rho0 gamma:E15', '5.2 < Mbc < 5.29 and abs(deltaE) < 2.0')

# Skim 1
# The new algorithm
skimfilter = register_module('SkimFilter')
skimfilter.set_name('SkimFilter_MyB2RhoGammaskim')
# Provide the particle lists that will be used as an event/skim filter
skimfilter.param('particleLists', ['B0'])
analysis_main.add_module(skimfilter)
# Suggest that you print a summary of the particle lists
summaryOfLists(['B0', 'rho0', 'gamma:E15', 'pi+:loose'])
# Create a new path for the skim output
myRhoGammaskim_path = create_path()
# The filter provides a boolean, which is true if any of the argument particle lists are not empty
skimfilter.if_value('=1', myRhoGammaskim_path, AfterConditionPath.CONTINUE)
# skim output contains Particles & Trash.PhysicsParticleLists along with mdst information
outputUdst('test_bck.udst.root', path=myRhoGammaskim_path)
outputMdst('test_bck.mdst.root', path=myRhoGammaskim_path)
# Process the events
process(analysis_main)

# print out the summary
print(statistics)
