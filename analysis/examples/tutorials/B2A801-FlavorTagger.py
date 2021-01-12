#!/usr/bin/env python3

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to include the flavor
# tagging user interface into your analysis.
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


# create path
cp_val_path = b2.Path()

# Environment of the MC or data sample
environmentType = "default"

# For Belle data/MC use
# from b2biiConversion import convertBelleMdstToBelleIIMdst
# import os

# os.environ['PGUSER'] = 'g0db'
# os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

# environmentType = "Belle"

# # You can use Belle MC/data as input calling this script as basf2 -i 'YourConvertedBelleData*.root' B2A801-FlavorTagger.py
# ma.inputMdstList(environmentType=environmentType, filelist=[], path=cp_val_path)


# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('analysis/mdst11_BGx1_b2jpsiks.root', 'validation', False),
             path=cp_val_path)


# Creates Muon particle list
ma.fillParticleList(decayString='mu+:all', cut='', path=cp_val_path)

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with dM<0.11
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=cp_val_path)


# For Belle data/MC use
#  # use the existent K_S0:mdst list to reconstruct B0 -> J/psi Ks decay
#  ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=cp_val_path)


# reconstruct Ks from standard pi+ particle list
ma.fillParticleList(decayString='pi+:all', cut='', path=cp_val_path)
ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=cp_val_path)

# reconstruct B0 -> J/psi Ks decay
ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=cp_val_path)

# Does the matching between reconstructed and MC particles
ma.matchMCTruth(list_name='B0:sig', path=cp_val_path)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name='B0:sig', fillWithMostLikely=True,
                    path=cp_val_path)

b2.conditions.append_globaltag(ma.getAnalysisGlobaltag())

# The default working directory is '.'
# Note that if you also train by yourself the weights of the trained Methods are saved therein.
# To save CPU time the weight files should be saved in the same server were you run.
#
# NEVER set uploadToDatabaseAfterTraining to True if you are not a librarian!!!
#
# BGx1 stays for MC generated with machine Background.
# The official weight files are trained using B0-> nu_tau anti-nu_tau as signal channel (no CP violation)
# to avoid that the flavor tagger learns asymmetries on the tag side.
# Only this kind of weight files are supported since release-03-01-00.

weightfiles = 'B2nunubarBGx1'

# Flavor Tagging Function. Default Expert mode to use the official weight files.
ft.flavorTagger(
    particleLists=['B0:sig'],
    weightFiles=weightfiles,
    path=cp_val_path)

# By default the flavorTagger trains and applies two methods, 'TMVA-FBDT' and 'FANN-MLP', for the combiner.
# If you want to train or test the Flavor Tagger only for one of them you have to specify it like:
#
# combinerMethods=['TMVA-FBDT']
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
# If you want to train yourself, have a look at the scripts under analysis/release-validation/CPVTools/
# in principle you need only to run CPVToolsValidatorInParalell.sh
# If you train the  event and combiner levels, you need two different samples of at least 500k events (one for each sampler).
# The different samples are needed to avoid biases between levels.
# We mean 500k of correctly corrected and MC matched neutral Bs. (isSignal > 0)
# You can also train the event level for all categories and then train the combiner
# for a specific combination (last two runs).
# It is also possible to train different combiners consecutively using the same weightFiles name.
# You just need always to specify the desired category combination while using the expert mode as:
#
# flavorTagger(particleLists=['B0:sig'], mode = 'Expert', weightFiles='B2JpsiKs_mu',
# categories=['Electron', 'Muon', 'Kaon', ... etc.])
#
# Another possibility is to train a combiner for a specific category combination using the default weight files
#
# Attention: to train the flavor tagger you need MC samples generated without built-in CP violation!
# The best sample for this is B0-> nu_tau anti-nu_tau .
# You can apply cuts using the flavor Tagger: isNAN(qrOutput(FBDT)) < 1 rejects all events which do not
# provide flavor information using the tag side
ma.applyCuts(list_name='B0:sig',
             cut='isNAN(qrOutput(FBDT)) < 1',
             path=cp_val_path)

# If you applied the cut on qrOutput(FBDT) > -2 before then you can rank by highest r- factor
ma.rankByHighest(particleList='B0:sig',
                 variable='abs(qrOutput(FBDT))',
                 numBest=0,
                 outputVariable='Dilution_rank',
                 path=cp_val_path)

# Fit vertex of the B0 on the signal side
vx.kFit(list_name='B0:sig', conf_level=0.0, decay_string='B0:sig -> [J/psi:mumu -> ^mu+ ^mu-] K_S0',
        constraint='', path=cp_val_path)


# Fit Vertex of the B0 on the tag side
vx.TagV(list_name='B0:sig', MCassociation='breco', path=cp_val_path)

# Select variables that will be stored to ntuple
fs_vars = vc.pid + vc.track + vc.track_hits + vc.mc_truth
jpsiandk0s_vars = vc.mc_truth
vertex_vars = vc.vertex + vc.mc_vertex + vc.kinematics + vc.mc_kinematics
bvars = vc.reco_stats + \
    vc.deltae_mbc + \
    vc.mc_truth + \
    vc.roe_multiplicities + \
    vc.tag_vertex + \
    vc.mc_tag_vertex + \
    vertex_vars

# Attention: the collection of flavor tagging variables is defined in the flavorTagger
bvars += ft.flavor_tagging

# Create aliases to save information for different particles
bvars = bvars + \
    vu.create_aliases_for_selected(list_of_variables=fs_vars,
                                   decay_string='B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
    vu.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                   decay_string='B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]') + \
    vu.create_aliases_for_selected(list_of_variables=vertex_vars,
                                   decay_string='B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]')

# Saving variables to ntuple
output_file = 'B2A801-FlavorTagger.root'
ma.variablesToNtuple(decayString='B0:sig',
                     variables=bvars,
                     filename=output_file,
                     treename='B0tree',
                     path=cp_val_path)

# Summary of created Lists
ma.summaryOfLists(particleLists=['J/psi:mumu', 'B0:sig'],
                  path=cp_val_path)

# Process the events
b2.process(cp_val_path)

# print out the summary
print(b2.statistics)

# If you want to calculate the efficiency of the FlavorTagger on your own
# File use the script analysis/release-validation/CPVTools/flavorTaggerEfficiency.py giving
# your file and the treename as arguments:

# basf2 flavorTaggerEfficiency.py 'YourFilesWithWildCards.root' 'B0tree'
