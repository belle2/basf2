#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to include the flavor
# tagging user interphase into your analysis.
# The following decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> K- pi+
#
# is the reconstructed B0. The qr value, i.e. the
# flavor*dilution factor of the not reconstructed B0,
# is saved as extraInfo to the reconstructed B0.
#
#
# Note: The weight files for the trained TMVA methods
# were produced using the signal MC sample created in
# MC campaign 3.5.
#
# Contributors: F. Abudinen & Moritz Gelb (February 2015)
#
######################################################

from basf2 import *
from modularAnalysis import *
from FlavorTagger import *
from stdFSParticles import *
from stdLooseFSParticles import *

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
reconstructDecay('K_S0:pipi -> pi+:loose pi-:loose', '0.25 < M < 0.75')

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with 3.0 < M(mumu) < 3.2 GeV
reconstructDecay('J/psi:mumu -> mu+:loose mu-:loose', '3.0 < M < 3.2')

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with 5.2 < M(J/PsiKs) < 5.4 GeV
reconstructDecay('B0:jspiks -> J/psi:mumu K_S0:pipi', '5.2 < M < 5.4')

# perform MC matching (MC truth asociation). Always before TagV
matchMCTruth('B0:jspiks')

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jspiks')

# Build Continuum suppression. Needed for Flavor Tagging Variables. (Dependency will be removed)
buildContinuumSuppression('B0:jspiks')

# Flavor Tagging Function. Later: Train or Test modus and Categories choice.
FlavorTagger(weightFiles='B2JpsiKs_mu')

# create and fill flat Ntuple with MCTruth, kinematic information and Flavor Tagger Output
toolsDST = ['EventMetaData', '^B0']
toolsDST += ['RecoStats', '^B0']
toolsDST += ['DeltaEMbc', '^B0']
toolsDST += ['CMSKinematics', '^B0']
toolsDST += ['MCHierarchy', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['PID', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['Track', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['MCTruth', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
toolsDST += ['FlavorTagging', '^B0']

# write out the flat ntuples
ntupleFile('B2A801-FlavorTagger.root')
ntupleTree('B0tree', 'B0:jspiks', toolsDST)

# Process the events
process(analysis_main)

# print out the summary
print statistics
