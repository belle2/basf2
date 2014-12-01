#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to save multiple particles
# as the following decay chain:
#
# D0 -> K- pi+
#
# rho0 -> pi+ pi-
#
# Note: This example uses the MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
# It also works on the Grid.
# Contributors: C-L Hsu (Dec 2014)
######################################################

from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from stdLooseFSParticles import *

set_log_level(LogLevel.INFO)
# gbasf2 setting
gb2_setuprel = 'build-2014-08-01'

# Run this tutorial either over signal MC or background MC (K*gamma)
# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/group/belle2/MC/signal/B2Rhogamma/mcprod1405/BGx1/mc35_B2Rhogamma_BGx1_s00/B2Rhogamma_e0001r001*_s00_BGx1.mdst.root'
     ]

# Run rho (pi+ pi-) reconstruction on B2Rhogamma MC
inputMdstList(filelistSIG)

# Run B0 -> rho gamma reconstruction over B0 -> K* gamma MC
# (uncomment next two lines and comment above two to run over BKG MC)
# rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-BKGMC.root'
# inputMdstList(filelistBKG)

# load all final state Particles
loadReconstructedParticles()

# use standard final state particle lists
#
# creates "gamma:highE" ParticleList
# contains all photon candidates with E>1.5 GeV
# stdHighEPhoton()

# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
stdLooseK()
# reconstruct D0 -> pi+ K- decay
# keep only candidates with 1.5 < M(pi+K-) < 2.2 GeV
reconDecay('D0 -> pi+:loose K-:loose', '1.5 < M < 2.2', True)
# reconstruct rho0 -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.2 GeV
reconDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.2', True)

# Skim 1
# The new algorithm
skimfilter = register_module('SkimFilter')
skimfilter.set_name('SkimFilter_D0skim')
# Provide the particle lists that will be used as an event/skim filter
skimfilter.param('particleLists', ['D0'])
analysis_main.add_module(skimfilter)
# Suggest that you print a summary of the particle lists
summaryOfLists(['D0', 'pi+:loose', 'K-:loose'])
# Create a new path for the skim output
D0skim_path = create_path()
# The filter provides a boolean, which is true if any of the argument particle lists are not empty
skimfilter.if_value('=1', D0skim_path, AfterConditionPath.CONTINUE)
# skim output contains Particles & Trash.PhysicsParticleLists along with mdst information
# Skim 2
skimfilter2 = register_module('SkimFilter')
skimfilter2.set_name('SkimFilter_rho0skim')
skimfilter2.param('particleLists', ['rho0'])
analysis_main.add_module(skimfilter2)
summaryOfLists(['rho0', 'pi+:loose', 'pi-:loose'])
rho0skim_path = create_path()
skimfilter2.if_value('=1', rho0skim_path, AfterConditionPath.CONTINUE)
outputUdst('D0+rho0_skim.udst.root')
outputMdst('D0+rho0_skim.mdst.root')

# Process the events
process(analysis_main)

# print out the summary
print statistics
