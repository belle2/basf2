#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<contact>Marco Milesi, marco.milesi@unimelb.edu.au</contact>
<description> This steering script fully reconstructs particle gun events for a set of charged stable particles,
and dumps ECL PID validation info in an ntuple (one for each pdgId).</description>
</header>
"""

import os
import basf2
from reconstruction import add_reconstruction
from ROOT import Belle2

# Pdg code of the charged stable particles & antiparticles.
chargedStableList = []
for idx in range(len(Belle2.Const.chargedStableSet)):
    particle = Belle2.Const.chargedStableSet.at(idx)
    if particle == Belle2.Const.deuteron:
        continue
    pdgId = particle.getPDGCode()
    chargedStableList.extend([pdgId, -pdgId])

# Create path.
main = basf2.create_path()

# Read input.
# Get it from the validation directory, otherwise try locally.
validation_dir = os.getenv("BELLE2_VALIDATION_DATA_DIR")
if validation_dir:
    inputFileName = os.path.join(validation_dir, "1000_PartGun_ChargedStable_EvtGenSim.root")
else:
    inputFileName = "1000_PartGun_ChargedStable_EvtGenSim.root"
    basf2.B2WARNING(f"Trying w/ {inputFileName} from the local ecl/validation/ directory (if any).")

if not os.path.exists(inputFileName):
    os.sys.exit(f"{inputFileName} not found!")

main.add_module("RootInput", inputFileName=inputFileName)

# Load parameters.
main.add_module("Gearbox")
# Create geometry.
main.add_module("Geometry")

# Reconstruct event.
add_reconstruction(main)
# Dump validation plots.
main.add_module("ECLChargedPIDDataAnalysisValidation", outputFileName="ECLChargedPid", inputPdgIdList=chargedStableList)

# Show progress of processing.
main.add_module("Progress")

basf2.print_path(main)

basf2.process(main)
