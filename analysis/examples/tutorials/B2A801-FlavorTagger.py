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
import variableCollections as vc
import variableCollectionsTools as vct
import stdCharged as stdc
from stdPi0s import stdPi0s

b2.B2FATAL("Input files for this tuttorial do not exist. \
    We are working on producing them.\
    Pleae feel free to use code snippets in a meanwhile.")

# check if the required input file exists
import os
if not os.path.isfile(os.getenv('BELLE2_EXAMPLES_DATA') + '/B2JPsiKs_JPsi2mumu.root'):
    b2.B2FATAL("You need the example data installed. Run `b2mount-tutorial-cloud` in terminal for it.")

# create path
my_path = ma.analysis_main

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename='$BELLE2_EXAMPLES_DATA/B2JPsiKs_JPsi2mumu.root',
             path=my_path)

# use standard final state particle lists
#
# creates "highPID" ParticleLists (and c.c.)
ma.fillParticleList(decayString='pi+:highPID',
                    cut='pionID > 0.5 and d0 < 5 and abs(z0) < 10',
                    path=my_path)
ma.fillParticleList(decayString='mu+:highPID',
                    cut='muonID > 0.2 and d0 < 2 and abs(z0) < 4',
                    path=my_path)


# reconstruct Ks -> pi+ pi- decay
# keep only candidates with dM<0.25
ma.reconstructDecay(decayString='K_S0:pipi -> pi+:highPID pi-:highPID',
                    cut='dM<0.25',
                    path=my_path)

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with dM<0.11
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:highPID mu-:highPID',
                    cut='dM<0.11 and 3.07 < M < 3.11',
                    path=my_path)

# reconstruct B0 -> J/psi Ks decay
# keep only candidates with Mbc > 5.1 and abs(deltaE)<0.15
ma.reconstructDecay(decayString='B0:jspiks -> J/psi:mumu K_S0:pipi',
                    cut='Mbc > 5.1 and abs(deltaE)<0.15',
                    path=my_path)

vx.vertexTree(list_name='B0:jspiks',
              conf_level=-1,  # keep all cadidates, 0:keep only fit survivors, optimise this cut for your need
              ipConstraint=True,
              # pins the B0 PRODUCTION vertex to the IP (increases SIG and BKG rejection) use for better vertex resolution
              updateAllDaughters=True,  # update momenta off ALL particles
              path=my_path
              )

# perform MC matching (MC truth asociation). Always before TagV
ma.matchMCTruth(list_name='B0:jspiks', path=my_path)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(list_name='B0:jspiks')

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
    particleLists=['B0:jspiks'],
    weightFiles='B2JpsiKs_muBGx1',
    path=my_path)

# NOTE: for Belle data, (i.e. b2bii users) should use modified line:
# ft.flavorTagger(
#     particleLists=['B0:jspiks'],
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
# flavorTagger(particleLists=['B0:jspiks'], mode = 'Sampler', weightFiles='B2JpsiKs_mu',
# categories=['Electron', 'Muon', 'Kaon', ... etc.]
# )
#
# After the Sampling process:
#
# flavorTagger(particleLists=['B0:jspiks'], mode = 'Teacher', weightFiles='B2JpsiKs_mu',
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
# flavorTagger(particleLists=['B0:jspiks'], mode = 'Expert', weightFiles='B2JpsiKs_mu',
# categories=['Electron', 'Muon', 'Kaon', ... etc.])
#
# Another possibility is to train a combiner for a specific category combination using the default weight files

# You can apply cuts using the flavor Tagger: qrOutput(FBDT) > -2 rejects all events which do not
# provide flavor information using the tag side
ma.applyCuts(list_name='B0:jspiks',
             cut='qrOutput(FBDT) > -2',
             path=my_path)

# If you applied the cut on qrOutput(FBDT) > -2 before then you can rank by highest r- factor
ma.rankByHighest(particleList='B0:jspiks',
                 variable='abs(qrOutput(FBDT))',
                 numBest=0,
                 outputVariable='Dilution_rank',
                 path=my_path)

# Fit Vertex of the B0 on the tag side
vx.TagV(list_name='B0:jspiks',
        MCassociation='breco',
        confidenceLevel=0.001,
        useFitAlgorithm='standard_PXD',
        path=my_path)

# Select variables that we want to store to ntuple
fs_vars = vc.pid + vc.track + vc.mc_truth + vc.mc_hierarchy
jpsiandk0s_vars = vc.mc_truth
bvars = vc.event_meta_data + \
    vc.reco_stats + \
    vc.deltae_mbc + \
    vc.ckm_kinematics + \
    vc.mc_truth + \
    vc.roe_multiplicities + \
    vc.flavor_tagging + \
    vc.tag_vertex + \
    vc.mc_tag_vertex + \
    vct.create_aliases_for_selected(list_of_variables=fs_vars,
                                    decay_string='B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
    vct.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                    decay_string='B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]')


# Saving variables to ntuple
output_file = 'B2A801-FlavorTagger.root'
ma.variablesToNtuple(decay_string='B0:jspiks',
                     variables=bvars,
                     filename=output_file,
                     treename='B0tree',
                     path=my_path)

# Summary of created Lists
ma.summaryOfLists(particleLists=['J/psi:mumu', 'K_S0:pipi', 'B0:jspiks'],
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
