#!/usr/bin/env python3
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
# The FlavorTagger already imports  modularAnalysis
from FlavorTagger import *
from stdFSParticles import *
from stdLooseFSParticles import *

# Add 10 signal MC files (each containing 1000 generated events)
filelistSIG = \
    [
        '/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000103/s00/e0000/4S/' +
        'r00000/1111440100/sub00/mdst_00000*_prod00000103_task0000000*.root'
    ]

inputMdstList(filelistSIG)

# use standard final state particle lists
#
# creates "highPID" ParticleLists (and c.c.)
fillParticleList('pi+:highPID', 'piid > 0.5 and d0 < 5 and abs(z0) < 10')
fillParticleList('mu+:highPID', 'muid > 0.2 and d0 < 2 and abs(z0) < 4')


# reconstruct Ks -> pi+ pi- decay
# keep only candidates with dM<0.25
reconstructDecay('K_S0:pipi -> pi+:highPID pi-:highPID', 'dM<0.25')
# fit K_S0 Vertex
fitVertex('K_S0:pipi', 0., '', 'rave', 'vertex', '', False)

# reconstruct J/psi -> mu+ mu- decay and fit vertex
# keep only candidates with dM<0.11
reconstructDecay('J/psi:mumu -> mu+:highPID mu-:highPID', 'dM<0.11')
applyCuts('J/psi:mumu', '3.07 < M < 3.11')
massVertexRave('J/psi:mumu', 0., '')

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with Mbc > 5.1 and abs(deltaE)<0.15
reconstructDecay('B0:jspiks -> J/psi:mumu K_S0:pipi', 'Mbc > 5.1 and abs(deltaE)<0.15')

# Fit the B0 Vertex
vertexRave('B0:jspiks', 0., 'B0 -> [J/psi -> ^mu+ ^mu-] K_S0', '')

# perform MC matching (MC truth asociation). Always before TagV
matchMCTruth('B0:jspiks')

# build the rest of the event associated to the B0
buildRestOfEvent('B0:jspiks')

# Before using the Flavor Tagger you need at least the default weight files for the employed release:
# copy the folder in @login.cc.kek.jp:
# scp -r /home/belle2/abudinen/public/FT-build-20xx-xx-xx/FlavorTagging
# into your workingDirectory/.
# The default working directory is '.'
# If you have an own analysis package it is recomended to use
# workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'.
# Note that if you also train by yourself the weights of the trained Methods are saved therein.
#
# Flavor Tagging Function. Default Expert mode to use the default weight files for the B2JpsiKs_mu channel.
# FlavorTagger(mode='Expert', weightFiles='B2JpsiKs_mu')
FlavorTagger(
    mode='Expert',
    weightFiles='B2JpsiKs_mu',
    combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
    workingDirectory=os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data',
    categories=[
        'Electron',
        'IntermediateElectron',
        'Muon',
        'IntermediateMuon',
        'KinLepton',
        'IntermediateKinLepton',
        'Kaon',
        'SlowPion',
        'FastPion',
        'Lambda',
        'FSC',
        'MaximumPstar',
        'KaonPion'],
    belleOrBelle2='Belle2')
#
# If you want to train the Flavor Tagger by yourself you have to specify the name of the weight files and the categories
# you want to use like:
#
# FlavorTagger(mode = 'Teacher', weightFiles='B2JpsiKs_mu', categories=['Electron', 'Muon', 'Kaon', ... etc.])
#
# Instead of the default name 'B2JpsiKs_mu' is better to use the abbreviation of your decay channel.
# If you do not specify any category combination, the default one (all categories) is choosed either in 'Teacher' or 'Expert' mode:
#
# All available categories are:
# [
# 'Electron',
# 'IntermediateElectron',
# 'Muon',
# 'IntermediateMuon',
# 'KinLepton',
# 'IntermediateKinLepton',
# 'Kaon',
# 'SlowPion',
# 'FastPion',
# 'Lambda',
# 'FSC',
# 'MaximumPstar',
# 'KaonPion']
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
#
# By default the FlavorTagger trains and applies two methods, 'TMVA-FBDT' and 'FANN-MLP', for the combiner.
# If you want to train or test the Flavor Tagger only for one of them you have to specify it like:
#
# combinerMethods=['TMVA-FBDT']
#
# With the belleOrBelle2 argument you specify if you are using Belle MC (also Belle Data) or Belle2 MC.
# If you want to use Belle MC please follow the dedicated tutorial B2A802-FlavorTagger-BelleMC.py
# since you need to follow a special module order.


# Fit Vertex of the B0 on the tag side
TagV('B0:jspiks', 'breco', 0.001, 'standard_PXD')

toolsDST = ['EventMetaData', '^B0']
toolsDST += ['RecoStats', '^B0']
toolsDST += ['DeltaEMbc', '^B0']
toolsDST += ['CMSKinematics', '^B0']
toolsDST += ['MCHierarchy', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['PID', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['Track', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
toolsDST += ['MCTruth', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
toolsDST += ['ROEMultiplicities', '^B0']
# create and fill flat Ntuple with MCTruth, kinematic information and Flavor Tagger Output
# Without any arguments only TMVA is saved. If you want to save the FANN Output please specify it.
# If you set qrCategories, the output of each category is saved.
toolsDST += ['FlavorTagging[TMVA, FANN, qrCategories]', '^B0']

toolsDST += ['TagVertex', '^B0']
toolsDST += ['DeltaT', '^B0']
toolsDST += ['MCTagVertex', '^B0']
toolsDST += ['MCDeltaT', '^B0']
# Note: The Ntuple Output is set to zero during training processes, i.e. when the 'Teacher' mode is used

# write out the flat ntuples
ntupleFile('B2A801-FlavorTagger.root')
ntupleTree('B0tree', 'B0:jspiks', toolsDST)

# Summary of created Lists
summaryOfLists(['J/psi:mumu', 'K_S0:pipi', 'B0:jspiks'])

# Process the events
process(analysis_main)

# print out the summary
print(statistics)

# If you want to calculate the efficiency of the FlavorTagger on your own
# File use the script analysis/examples/FlavorTaggerEfficiency.py giving
# your file as argument:

# basf2 FlavorTaggerEfficiency.py YourFile.root

# Note: This efficiency script needs MCParticles. If the name of your tree is not 'B0tree' please change line 65.
