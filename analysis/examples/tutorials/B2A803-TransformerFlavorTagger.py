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
# This tutorial demonstrates how to include the transformer flavor       #
# tagging (TFlaT) user interface into your analysis.                     #
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
import tflat.flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu
import stdV0s as stdV0s


# create path
main = b2.Path()

# Environment of the MC or data sample
environmentType = "default"


# load input ROOT file
ma.inputMdst(environmentType=environmentType,
             filename=b2.find_file('analysis/mdst11_BGx1_b2jpsiks.root', 'validation', False),
             path=main)


# Creates Muon particle list
ma.fillParticleList(decayString='mu+:all', cut='', path=main)

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with dM<0.11
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=main)


# reconstruct Ks
stdV0s.stdKshorts(path=main)

# reconstruct B0 -> J/psi Ks decay
ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:merged', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=main)

# Does the matching between reconstructed and MC particles
ma.matchMCTruth(list_name='B0:sig', path=main)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name='B0:sig', fillWithMostLikely=True,
                    path=main)

b2.conditions.append_globaltag(ma.getAnalysisGlobaltag())


# TFlaT Flavor Tagging Function. Do not set uniqueIdentifier to get the official weight files for used release.
ft.flavorTagger(
    particleLists=['B0:sig'],
    path=main)

# You can apply cuts using the flavor Tagger: isNAN(qrTFLAT) < 1 rejects all events which do not
# provide flavor information using the tag side
ma.applyCuts(list_name='B0:sig',
             cut='isNAN(qrTFLAT) < 1',
             path=main)

# You can rank by highest r- factor
ma.rankByHighest(particleList='B0:sig',
                 variable='abs(qrTFLAT)',
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

# Add FT output to variables that will be saved
bvars += ['qrTFLAT']

# Create aliases to save information for different particles
bvars = bvars + \
    vu.create_aliases_for_selected(list_of_variables=fs_vars,
                                   decay_string='B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
    vu.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                   decay_string='B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]') + \
    vu.create_aliases_for_selected(list_of_variables=vertex_vars,
                                   decay_string='B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]')

# Saving variables to ntuple
output_file = 'B2A803-FlavorTagger.root'
ma.variablesToNtuple(decayString='B0:sig',
                     variables=bvars,
                     filename=output_file,
                     treename='B0tree',
                     path=main)

# Summary of created Lists
ma.summaryOfLists(particleLists=['J/psi:mumu', 'B0:sig'],
                  path=main)

# Process the events
b2.process(main, calculateStatistics=True)
