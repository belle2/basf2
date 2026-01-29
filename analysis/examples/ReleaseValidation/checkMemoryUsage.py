#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
#
# This script tests the memory usage of the analysis package for
# validation purposes. It uses the following decay chain:
#
# B0 -> J/psi Ks
#        |    |
#        |    +-> pi+ pi-
#        |
#        +-> mu- mu+
#
#####################################################################

import argparse
import basf2 as b2
import modularAnalysis as ma
import stdV0s
import flavorTagger as ft
import vertex as vx
import variables.collections as vc
import variables.utils as vu

b2.set_random_seed("aSeed")

# create path
main = b2.Path()

parser = argparse.ArgumentParser(
    description="Script to chech memory usage as part of light release validaiton"
)

parser.add_argument(
    "--release", type=str, help="The light release that is being tested."
)
args = parser.parse_args()

# load input ROOT file
ma.inputMdst(filename="", path=main)

# create muon particle list
ma.fillParticleList(decayString="mu+:all", cut="", path=main)

# reconstruct J/psi -> mu+ mu- decay
# keep only candidates with dM<0.11
ma.reconstructDecay(
    decayString="J/psi:mumu -> mu+:all mu-:all", cut="dM<0.11", path=main
)

# load KS from standard particle list
stdV0s.stdKshorts(fitter="TreeFit", path=main)

# reconstruct B0 -> J/psi KS decay
ma.reconstructDecay(
    decayString="B0:sig -> J/psi:mumu K_S0:merged",
    cut="Mbc > 5.2 and abs(deltaE)<0.15",
    path=main,
)

# match reconstructed with MC particles
ma.matchMCTruth(list_name="B0:sig", path=main)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name="B0:sig", path=main)

# set analysis global tag
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Flavor Tagging
ft.flavorTagger(
    particleLists=["B0:sig"], weightFiles="B2nunubarBGx1", path=main, useGNN=True
)

# rank by highest r- factor
ma.rankByHighest(
    particleList="B0:sig",
    variable="abs(qrOutput(FBDT))",
    numBest=0,
    outputVariable="Dilution_rank",
    path=main,
)

# fit B vertex on the signal side
vx.treeFit(
    list_name="B0:sig",
    conf_level=-1,
    massConstraint=["K_S0"],
    ipConstraint=True,
    updateAllDaughters=True,
    path=main,
)

# fit B vertex on the tag side
vx.TagV(list_name="B0:sig", MCassociation="breco", path=main)

# select variables that will be stored to ntuple
fs_vars = vc.pid + vc.track + vc.track_hits + vc.mc_truth
jpsiandk0s_vars = vc.mc_truth
vertex_vars = vc.vertex + vc.mc_vertex + vc.kinematics + vc.mc_kinematics
bvars = (
    vc.reco_stats
    + vc.deltae_mbc
    + vc.mc_truth
    + vc.roe_multiplicities
    + vc.tag_vertex
    + vc.mc_tag_vertex
    + vertex_vars
)

# Attention: the collection of flavor tagging variables is defined in the flavorTagger
bvars += ft.flavor_tagging

# Create aliases to save information for different particles
bvars = (
    bvars
    + vu.create_aliases_for_selected(
        list_of_variables=fs_vars,
        decay_string="B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]",
    )
    + vu.create_aliases_for_selected(
        list_of_variables=jpsiandk0s_vars,
        decay_string="B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]",
    )
    + vu.create_aliases_for_selected(
        list_of_variables=vertex_vars,
        decay_string="B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]",
    )
)

# saving variables to ntuple
output_file = f"output/{args.release}.root"
ma.variablesToNtuple(
    decayString="B0:sig",
    variables=bvars,
    filename=output_file,
    treename="tree",
    path=main,
)

# summary of created Lists
ma.summaryOfLists(particleLists=["K_S0:merged", "J/psi:mumu", "B0:sig"], path=main)

# profile execution time and memory usage
main.add_module(
    "Profile",
    outputFileName=f"output/MemoryUsage_{args.release}.root",
    rssOutputFileName=f"output/RSSMemoryUsage_{args.release}.root",
)

# process the events
b2.process(main)

# print out the summary
print(b2.statistics)
