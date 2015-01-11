#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to perform Vertex fits
# using Rave and how to use TagV and save Delta t.
# The following  decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> K- pi+
#
# is reconstructed B0 vertex is fitted using the two m
# the side vertex is fittes and Delta t (in ps.) is
# calculated
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: L. Li Gioi (October 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import vertexRave
from modularAnalysis import buildRestOfEvent
from modularAnalysis import TagV
from stdLooseFSParticles import stdLoosePi
from stdLooseFSParticles import stdLooseMu

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    ['/group/belle2/MC/signal/B2JpsiKs_mu/mcprod1405/BGx1/mc35_B2JpsiKs_mu_BGx1_s00/B2JpsiKs_mu_e0001r001*_s00_BGx1.mdst.root'
     ]

inputMdstList(filelistSIG)

# use standard final state particle lists
#
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "mu+:loose" ParticleList (and c.c.)
stdLooseMu()

# reconstruct Ks -> pi+ pi- decay
# keep only candidates with 0.4 < M(pipi) < 0.6 GeV
reconstructDecay('K_S0:pipi -> pi+:loose pi-:loose', '0.4 < M < 0.6')

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with 3.0 < M(mumu) < 3.2 GeV
reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', '3.0 < M < 3.2')

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with 5.2 < M(J/PsiKs) < 5.4 GeV
reconstructDecay('B0:jspiks -> J/psi:mumu K_S0:pipi', '5.2 < M < 5.4')

# perform B0 kinematic vertex fit using only the mu+ mu-
# keep candidates only passing C.L. value of the fit > 0.0 (no cut)
vertexRave('B0:jspiks', 0.0, 'B0 -> [J/psi -> ^mu+ ^mu-] K_S0')

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jspiks')

# perform MC matching (MC truth asociation). Always before TagV
matchMCTruth('B0:jspiks')

# calculate the Tag Vertex and Delta t (in ps)
# breco: type of MC association.
TagV('B0:jspiks', 'breco')

# create and fill flat Ntuple with MCTruth, kinematic information and D0 FlightInfo
toolsDST = ['EventMetaData', '^B0']
toolsDST += ['InvMass[BeforeFit]',
             '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
toolsDST += ['DeltaEMbc', '^B0']
toolsDST += ['CMSKinematics', '^B0']
toolsDST += ['Vertex', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
toolsDST += ['MCVertex', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
toolsDST += ['PID', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['Track', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['MCTruth', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
toolsDST += ['TagVertex', '^B0']
toolsDST += ['MCTagVertex', '^B0']
toolsDST += ['DeltaT', '^B0']
toolsDST += ['MCDeltaT', '^B0']

# write out the flat ntuples
ntupleFile('B2A410-TagVertex.root')
ntupleTree('B0tree', 'B0:jspiks', toolsDST)

# Process the events
process(analysis_main)

# print out the summary
print statistics
