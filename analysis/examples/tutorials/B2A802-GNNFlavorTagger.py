#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to include the GNN flavor               #
# tagging (GFlaT) user interface into your analysis.                     #
# The following decay chain:                                             #
#                                                                        #
# B0 -> J/psi Ks                                                         #
#        |    |                                                          #
#        |    +-> pi+ pi-                                                #
#        |                                                               #
#        +-> mu- mu+                                                     #
#                                                                        #
# is the reconstructed B0. The qr value, i.e. the                        #
# flavor*dilution factor of the not reconstructed B0,                    #
# is saved as extraInfo to the reconstructed B0.                         #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu


# create path
main = b2.Path()

# Environment of the MC or data sample
environmentType = "default"

# For Belle data/MC use
# from b2biiConversion import convertBelleMdstToBelleIIMdst
# import os

# os.environ['PGUSER'] = 'g0db'
# os.environ['USE_GRAND_REPROCESS_DATA'] = '1'

# environmentType = "Belle"

# # You can use Belle MC/data as input calling this script as basf2 -i 'YourConvertedBelleData*.root' B2A802-FlavorTagger.py
# ma.inputMdstList(environmentType=environmentType, filelist=[], path=main)

# load input ROOT file
ma.inputMdst(environmentType=environmentType,
             filename=b2.find_file('analysis/mdst11_BGx1_b2jpsiks.root', 'validation', False),
             path=main)


# Creates Muon particle list
ma.fillParticleList(decayString='mu+:all', cut='', path=main)

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with dM<0.11
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=main)


# For Belle data/MC use
#  # use the existent K_S0:mdst list to reconstruct B0 -> J/psi Ks decay
#  ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu  K_S0:mdst', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=main)


# reconstruct Ks from standard pi+ particle list
ma.fillParticleList(decayString='pi+:all', cut='', path=main)
ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=main)

# reconstruct B0 -> J/psi Ks decay
ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=main)

# Does the matching between reconstructed and MC particles
ma.matchMCTruth(list_name='B0:sig', path=main)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name='B0:sig', fillWithMostLikely=True,
                    path=main)

b2.conditions.append_globaltag(ma.getAnalysisGlobaltag())


# GNN Flavor Tagging Function. Default Expert mode to use the official weight files.
ft.flavorTagger(
    particleLists=['B0:sig'],
    useGNN=True,
    path=main)

# You can apply cuts using the flavor Tagger: isNAN(qrGNN) < 1 rejects all events which do not
# provide flavor information using the tag side
ma.applyCuts(list_name='B0:sig',
             cut='isNAN(qrGNN) < 1',
             path=main)

# If you applied the cut on qrGNN > -2 before then you can rank by highest r- factor
ma.rankByHighest(particleList='B0:sig',
                 variable='abs(qrGNN)',
                 numBest=0,
                 outputVariable='Dilution_rank',
                 path=main)

# Fit vertex of the B0 on the signal side
vx.kFit(list_name='B0:sig', conf_level=0.0, decay_string='B0:sig -> [J/psi:mumu -> ^mu+ ^mu-] K_S0',
        constraint='', path=main)


# Fit Vertex of the B0 on the tag side
vx.TagV(list_name='B0:sig', MCassociation='breco', path=main)

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
output_file = 'B2A802-FlavorTagger.root'
ma.variablesToNtuple(decayString='B0:sig',
                     variables=bvars,
                     filename=output_file,
                     treename='B0tree',
                     path=main)

# Summary of created Lists
ma.summaryOfLists(particleLists=['J/psi:mumu', 'B0:sig'],
                  path=main)

# Process the events
b2.process(main)

# print out the summary
print(b2.statistics)
