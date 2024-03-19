#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>PartGunChargedStableGenSim.root</input>
  <contact>Marcel Hohmann, mhohmann@student.unimelb.edu.au</contact>
  <description> This steering script fully reconstructs particle gun events for a set of charged stable particles,
and dumps ECL PID validation info in an ntuple and a set of histograms (one file for each pdgId).</description>
</header>
"""

import basf2
from reconstruction import add_reconstruction
from ROOT import Belle2

# Pdg code of the charged stable particles & antiparticles.
chargedStableList = []
for idx in range(len(Belle2.Const.chargedStableSet)):
    particle = Belle2.Const.chargedStableSet.at(idx)
    # Skip deuteron for now...
    if particle == Belle2.Const.deuteron:
        continue
    pdgId = particle.getPDGCode()
    chargedStableList.extend([pdgId, -pdgId])


# Merge particles and antiparticles in the same plots for these hypotheses.
mergeChargeOfPdgIds = [
    Belle2.Const.pion.getPDGCode(),
    Belle2.Const.kaon.getPDGCode(),
    Belle2.Const.proton.getPDGCode()
]

# Create path.
main = basf2.create_path()

# Read input.
inputFileName = "../PartGunChargedStableGenSim.root"
main.add_module("RootInput", inputFileName=inputFileName)

# Load parameters.
main.add_module("Gearbox")
# Create geometry.
main.add_module("Geometry")

# Reconstruct events.
add_reconstruction(main)

# Dump validation plots.
main.add_module(
    "ECLChargedPIDDataAnalysisValidation",
    outputFileName="ECLChargedPid",
    inputPdgIdList=chargedStableList,
    mergeChargeOfPdgIds=mergeChargeOfPdgIds)

# Show progress of processing.
main.add_module("ProgressBar")

basf2.print_path(main)

main.add_module('Progress')
basf2.process(main)
print(basf2.statistics)
