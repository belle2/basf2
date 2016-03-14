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

# Before using the Flavor Tagger you need at least the default weight files: copy the folder in @login.cc.kek.jp:
# scp -r /home/belle2/abudinen/public/FlavorTagging
# into your local release under analysis/data/.
#
# Flavor Tagging Function. Default Expert mode to use the default weight files for the B2JpsiKs_mu channel.
# FlavorTagger(mode='Expert', weightFiles='B2JpsiKs_mu')
FlavorTagger(
    recoParticle='B0:jspiks',
    mode='Expert',
    weightFiles='B2JpsiKs_mu',
    categories=['Electron', 'Muon', 'KinLepton', 'Kaon', 'SlowPion', 'FastPion', 'Lambda', 'FSC', 'MaximumP*', 'KaonPion'])
#
# If you want to train the Flavor Tagger by yourself you have to specify the name of the weight files and the categories
# you want to use like:
#
# FlavorTagger(mode = 'Teacher', weightFiles='B2JpsiKs_mu', categories=['Electron', 'Muon', 'Kaon', ... etc.])
#
# Instead of the default name 'B2JpsiKs_mu' is better to use the abbreviation of your decay channel.
# If you do not specify any category combination, the default one is choosed either in 'Teacher' or 'Expert' mode:
#
# ['Electron', 'Muon', 'KinLepton', 'Kaon', 'SlowPion', 'FastPion', 'Lambda', 'FSC', 'MaximumP*', 'KaonPion']
#
# All available categories are:
# ['Electron', 'IntermediateElectron', 'Muon', 'IntermediateMuon', 'KinLepton', 'Kaon', 'SlowPion', 'FastPion',
# 'Lambda', 'FSC', 'MaximumP*', 'KaonPion']
#
# If you train by yourself you need to run this file 3 times (in order to train track, event and combiner levels)
# with 3 different samples of 500k events.
# Three different 500k events samples are needed in order to avoid biases between levels.
# You can also train track and event level for all categories (1st and 2nd runs) and then train the combiner
# for a specific combination (3rd run).
# It is also possible to train different combiners consecutively using the same weightFiles name.
# You just need always to specify the desired category combination while using the expert mode as:
#
# FlavorTagger(mode = 'Expert', weightFiles='B2JpsiKs_mu', categories=['Electron', 'Muon', 'Kaon', ... etc.])
#
# Another possibility is to train a combiner for a specific category combination using the default weight files

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
#  Note: The Ntuple Output is set to zero during training processes, i.e. when the 'Teacher' mode is used

# write out the flat ntuples
ntupleFile('B2A801-FlavorTagger.root')
ntupleTree('B0tree', 'B0:jspiks', toolsDST)

# Process the events
process(analysis_main)

# print out the summary
print statistics
