#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> rho gamma
#       |
#       +-> pi+ pi-
#
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdFSParticles import stdHighEPhoton
from stdCharged import stdLoosePi

# Run this tutorial either over signal MC or background MC (K*gamma)
# Add 10 signal MC files (each containing 1000 generated events)
filelistBKG = [
    '/hsm/belle2/bdata/MC/signal/B2Kstargamma/mcprod1405/BGx1/mc35_B2Kstargamma_BGx1_s00/B2Kstargamma_e0001r001*_s00_BGx1.mdst.root'
]
filelistSIG = \
    ['/hsm/belle2/bdata/MC/signal/B2Rhogamma/mcprod1405/BGx1/mc35_B2Rhogamma_BGx1_s00/B2Rhogamma_e0001r001*_s00_BGx1.mdst.root'
     ]

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma MC
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-SIGMC.root'
inputMdstList('MC5', filelistSIG)

# Run B0 -> rho gamma reconstruction over B0 -> K* gamma MC
# (uncomment next two lines and comment above two to run over BKG MC)
# rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-BKGMC.root'
# inputMdstList('MC5',filelistBKG)

# use standard final state particle lists
#
# creates "gamma:highE" ParticleList
# contains all photon candidates with E>1.5 GeV
stdHighEPhoton()

# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
reconstructDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.0')

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
reconstructDecay('B0 -> rho0 gamma:highE', '5.2 < Mbc < 5.29 and abs(deltaE) < 2.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0')

# create and fill flat Ntuple with MCTruth and kinematic information
toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['Kinematics', '^B0 -> ^rho0 ^gamma']
toolsB0 += ['InvMass', 'B0 -> ^rho0 gamma']
toolsB0 += ['DeltaEMbc', '^B0']
toolsB0 += ['PID', 'B0 -> [rho0 -> ^pi+ ^pi-] gamma']
toolsB0 += ['Track', 'B0 -> [rho0 -> ^pi+ ^pi-] gamma']
toolsB0 += ['Cluster', 'B0 -> ^rho0 ^gamma']
toolsB0 += ['MCTruth', '^B0 -> ^rho0 ^gamma']

# write out the flat ntuple
ntupleFile(rootOutputFile)
ntupleTree('b0', 'B0', toolsB0)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
