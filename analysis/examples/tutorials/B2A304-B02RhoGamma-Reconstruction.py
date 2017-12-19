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
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (Demeber 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdFSParticles import stdPhotons
from stdCharged import stdLoosePi

# Run this tutorial either over signal MC or background MC (K*gamma)
# Add 10 signal MC files (each containing 1000 generated events)
filelistBKG = ['/hsm/belle/bdata2/users/ikomarov/tutorial_samples/Bd_Kstgamma_GENSIMRECtoDST.dst.root']
filelistSIG = ['/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002326/e0000/\
4S/r00000/1110021010/sub00/mdst_000001_prod00002326_task00000001.root']

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma MC
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-SIGMC.root'
inputMdstList('default', filelistSIG)

# Run B0 -> rho gamma reconstruction over B0 -> K* gamma MC
# (uncomment next two lines and comment above two to run over BKG MC)
# rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction-BKGMC.root'
# inputMdstList('default',filelistBKG)

# use standard final state particle lists
#
# creates "gamma:tight" ParticleList
stdPhotons('tight')

# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
reconstructDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.0')

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
reconstructDecay('B0 -> rho0 gamma:tight', '5.2 < Mbc < 5.29 and abs(deltaE) < 2.0')

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
