#!/usr/bin/env python
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <output>CPVToolsOutput.root</output>
  <contact>Thibaud Humair; thumair@mpp.mpg.de</contact>
  <description>This file employs all the time dependent CPV analysis tools. It reconstructs B0sig->J/PsiKs on the signal side
  and applies the flavor tagger on the ROE. The vertex of B0sig is reconstructed and the vertex of B0tag is reconstructed
  with the TagV module. </description>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu

# create path
cp_val_path = b2.Path()

inputFile = b2.find_file(filename='analysis/mdst11_BGx1_b2jpsiks.root', data_type='validation')
ma.inputMdst(environmentType='default', filename=inputFile, path=cp_val_path)

# ma.inputMdst(environmentType='default', filename='../mdst11_BGx1_b2jpsiks.root', path=cp_val_path)

# Reconstruction of signal side and MC match
ma.fillParticleList(decayString='pi+:all', cut='', path=cp_val_path)
ma.fillParticleList(decayString='mu+:all', cut='', path=cp_val_path)

ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=cp_val_path)
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=cp_val_path)
ma.reconstructDecay(decayString='B0:jpsiks -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.2', path=cp_val_path)

ma.matchMCTruth(list_name='B0:jpsiks', path=cp_val_path)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name='B0:jpsiks', fillWithMostLikely=True, path=cp_val_path)

# Get Special GT for the flavor tagger weight files
b2.conditions.append_globaltag(ma.getAnalysisGlobaltag())

# Flavor Tagger, Vertex of Signal Side and TagV
ft.flavorTagger(
    particleLists=['B0:jpsiks'],
    weightFiles='B2nunubarBGx1',
    path=cp_val_path)

vx.raveFit(list_name='B0:jpsiks', conf_level=0.0,
           decay_string='B0:jpsiks -> [J/psi:mumu -> ^mu+ ^mu-] K_S0', constraint='', path=cp_val_path)
vx.TagV(list_name='B0:jpsiks', MCassociation='breco', path=cp_val_path)

# Select variables that will be stored to ntuple
fs_vars = vc.pid + vc.track + vc.track_hits + vc.mc_truth
jpsiandk0s_vars = vc.mc_truth
vertex_vars = vc.vertex + vc.mc_vertex + vc.kinematics + vc.mc_kinematics
bvars = vc.reco_stats + \
    vc.deltae_mbc + \
    vc.mc_truth + \
    vc.roe_multiplicities + \
    ft.flavor_tagging + \
    vc.tag_vertex + \
    vc.mc_tag_vertex + \
    vertex_vars + \
    vu.create_aliases_for_selected(list_of_variables=fs_vars,
                                   decay_string='B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]') + \
    vu.create_aliases_for_selected(list_of_variables=jpsiandk0s_vars,
                                   decay_string='B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]') + \
    vu.create_aliases_for_selected(list_of_variables=vertex_vars,
                                   decay_string='B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]')

# Saving variables to ntuple
ma.variablesToNtuple(decayString='B0:jpsiks',
                     variables=bvars,
                     filename='../CPVToolsOutput.root',
                     treename='B0tree',
                     path=cp_val_path)

ma.summaryOfLists(particleLists=['B0:jpsiks'], path=cp_val_path)

# Process the events
b2.process(cp_val_path)

# print out the summary
print(b2.statistics)
