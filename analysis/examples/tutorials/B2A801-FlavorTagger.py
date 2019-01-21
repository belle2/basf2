#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to include the flavor
# tagging user interphase into your analysis.
# The following decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> mu- mu+
#
# is the reconstructed B0. The qr value, i.e. the
# flavor*dilution factor of the not reconstructed B0,
# is saved as extraInfo to the reconstructed B0.
#
# Contributors: F. Abudinen & Moritz Gelb (February 2015)
#               I. Komarov (September 2018)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu
import stdCharged as stdc
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2JPsipi0_JPsi2mumu.root', 'examples', False),
             path=my_path)

# use standard final state particle lists
#
# creates "pi0:looseFit" ParticleList
# https://confluence.desy.de/display/BI/Physics+StandardParticles
stdPi0s(listtype='looseFit', path=my_path)
# creates "highPID" ParticleLists (and c.c.)
ma.fillParticleList(decayString='mu+:highPID',
                    cut='muonID > 0.2 and d0 < 2 and abs(z0) < 4',
                    path=my_path)


# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with dM<0.11
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:highPID mu-:highPID',
                    cut='dM<0.11 and 3.07 < M < 3.11',
                    path=my_path)

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with Mbc > 5.1 and abs(deltaE)<0.15
ma.reconstructDecay(decayString='B0:jspipi0 -> J/psi:mumu pi0:looseFit',
                    cut='Mbc > 5.1 and abs(deltaE)<0.15',
                    path=my_path)

vx.vertexTree(list_name='B0:jspipi0',
              conf_level=-1,  # keep all cadidates, 0:keep only fit survivors, optimise this cut for your need
              ipConstraint=True,
              # pins the B0 PRODUCTION vertex to the IP (increases SIG and BKG rejection) use for better vertex resolution
              updateAllDaughters=True,  # update momenta off ALL particles
              path=my_path
              )

# perform MC matching (MC truth asociation). Always before TagV
ma.matchMCTruth(list_name='B0:jspipi0', path=my_path)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name='B0:jspipi0',
                    path=my_path)

# Before using the Flavor Tagger you need at least the default weight files. If you do not set
# any parameter the flavorTagger downloads them automatically from the database.
# You just have to use a special global tag of the conditions database. Check in
# https://confluence.desy.de/display/BI/Physics+FlavorTagger
# E.g. for release-00-09-01

# use_central_database("GT_gen_prod_003.11_release-00-09-01-FEI-a")

# The default working directory is '.'
# If you have an own analysis package it is recomended to use
# workingDirectory = os.environ['BELLE2_LOCAL_DIR'] + '/analysis/data'.
# Note that if you also train by yourself the weights of the trained Methods are saved therein.
# To save CPU time the weight files should be saved in the same server were you run.
#
# NEVER set uploadToDatabaseAfterTraining to True if you are not a librarian!!!
#
# Flavor Tagging Function. Default Expert mode to use the default weight files for the B2JpsiKs_mu channel.
ft.flavorTagger(
    particleLists=['B0:jspipi0'],
    weightFiles='B2JpsiKs_muBGx1',
    path=my_path)

# NOTE: for Belle data, (i.e. b2bii users) should use modified line:
# ft.flavorTagger(
#     particleLists=['B0:jspipi0'],
#     combinerMethods=['TMVA-FBDT', 'FANN-MLP'],
#     belleOrBelle2='Belle')

#
# BGx0 stays for MC generated without machine Background.
# Please use B2JpsiKs_muBGx1 if you use MC generated with machine background.
# By default the flavorTagger trains and applies two methods, 'TMVA-FBDT' and 'FANN-MLP', for the combiner.
# If you want to train or test the Flavor Tagger only for one of them you have to specify it like:
#
# combinerMethods=['TMVA-FBDT']
#
# With the belleOrBelle2 argument you specify if you are using Belle MC (also Belle Data) or Belle2 MC.
# If you want to use Belle MC please follow the dedicated tutorial B2A802-FlavorTagger-BelleMC.py
# since you need to follow a special module order.
#
# _______________________________________________________________________________________________________________
# The following will be updated in a new Tutorial:
# If you want to train the Flavor Tagger by yourself you have to specify the name of the weight files and the categories
# you want to use like:
#
# flavorTagger(particleLists=['B0:jspipi0'], mode = 'Sampler', weightFiles='B2JpsiKs_mu',
# categories=['Electron', 'Muon', 'Kaon', ... etc.]
# )
#
# After the Sampling process:
#
# flavorTagger(particleLists=['B0:jspipi0'], mode = 'Teacher', weightFiles='B2JpsiKs_mu',
# categories=['Electron', 'Muon', 'Kaon', ... etc.]
# )
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
# 'FastHadron',
# 'Lambda',
# 'FSC',
# 'MaximumPstar',
# 'KaonPion']
#
# If you train by yourself you need to run this file 6 times alternating between "Sampler" and "Teacher" modes
# in order to train event and combiner levels.
# with 3 different samples of at least 500k events (one for each sampler).
# Three different 500k events samples are needed in order to avoid biases between levels.
# We mean 500k of correctly corrected and MC matched neutral Bs. (isSignal > 0)
# You can also train track and event level for all categories (1st to 4th runs) and then train the combiner
# for a specific combination (last two runs).
# It is also possible to train different combiners consecutively using the same weightFiles name.
# You just need always to specify the desired category combination while using the expert mode as:
#
# flavorTagger(particleLists=['B0:jspipi0'], mode = 'Expert', weightFiles='B2JpsiKs_mu',
# categories=['Electron', 'Muon', 'Kaon', ... etc.])
#
# Another possibility is to train a combiner for a specific category combination using the default weight files

# You can apply cuts using the flavor Tagger: qrOutput(FBDT) > -2 rejects all events which do not
# provide flavor information using the tag side
ma.applyCuts(list_name='B0:jspipi0',
             cut='qrOutput(FBDT) > -2',
             path=my_path)

# If you applied the cut on qrOutput(FBDT) > -2 before then you can rank by highest r- factor
ma.rankByHighest(particleList='B0:jspipi0',
                 variable='abs(qrOutput(FBDT))',
                 numBest=0,
                 outputVariable='Dilution_rank',
                 path=my_path)

# Fit Vertex of the B0 on the tag side
vx.TagV(list_name='B0:jspipi0',
        MCassociation='breco',
        confidenceLevel=0.001,
        useFitAlgorithm='standard_PXD',
        path=my_path)

# Select variables that we want to store to ntuple
fs_vars = vc.pid + vc.track + vc.mc_truth
jpsiandk0s_vars = vc.mc_truth
bvars = vc.reco_stats + \
    vc.deltae_mbc + \
    vc.mc_truth + \
    vc.roe_multiplicities + \
    vc.flavor_tagging + \
    vc.tag_vertex + \
    vc.mc_tag_vertex + \
    vu.create_aliases_for_selected(list_of_variables=fs_vars,
                                   decay_string='B0 -> [J/psi -> ^mu+ ^mu-] pi0') + \
    vu.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                   decay_string='B0 -> [^J/psi -> mu+ mu-] ^pi0')


# Saving variables to ntuple
output_file = 'B2A801-FlavorTagger.root'
ma.variablesToNtuple(decayString='B0:jspipi0',
                     variables=bvars,
                     filename=output_file,
                     treename='B0tree',
                     path=my_path)

# Summary of created Lists
ma.summaryOfLists(particleLists=['J/psi:mumu', 'pi0:looseFit', 'B0:jspipi0'],
                  path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)

# If you want to calculate the efficiency of the FlavorTagger on your own
# File use the script analysis/examples/FlavorTaggerEfficiency.py giving
# your file as argument:

# basf2 FlavorTaggerEfficiency.py YourFile.root

# Note: This efficiency script needs MCParticles. If the name of your tree is not 'B0tree' please change line 65.
