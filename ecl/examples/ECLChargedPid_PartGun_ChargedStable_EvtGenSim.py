#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<contact>Marco Milesi, marco.milesi@unimelb.edu.au</contact>
<description> This steering script generates 1000 particle gun events for a set of charged stable particles,
runs the fullsim and dumps info in a file.</description>
</header>
"""

import os
import glob
import basf2
from simulation import add_simulation
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

# Event setting and info.
main.add_module("EventInfoSetter", evtNumList=[1000], runList=[1])

# Fixed random seed.
basf2.set_random_seed("P1s@Merd@")

# Single particle generator settings.
pGun = basf2.register_module("ParticleGun")
param_pGun = {
    "pdgCodes": chargedStableList,
    "nTracks": 0,  # 0 : generate 1 track per pdgId per event.
    "momentumGeneration": "uniform",
    "momentumParams": [0.1, 5.0],
    "thetaGeneration": "uniform",
    "thetaParams": [17., 150.],
    "phiGeneration": "uniform",
    "phiParams": [0, 360],
    "vertexGeneration": "uniform",
    "xVertexParams": [0.0, 0.0],
    "yVertexParams": [0.0, 0.0],
    "zVertexParams": [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

# Load parameters.
main.add_module("Gearbox")
# Create geometry.
main.add_module("Geometry")

bg_dir = os.getenv("BELLE2_BACKGROUND_DIR")
if bg_dir:
    bg = glob.glob(f"{bg_dir}/*.root")
else:
    basf2.B2WARNING("Env variable BELLE2_BACKGROUND_DIR is not set.")
    bg = None

# Detector simulation.
add_simulation(main, bkgfiles=bg)

# Create output of event generation + detector simulation.
main.add_module("RootOutput", outputFileName="1000_PartGun_ChargedStable_EvtGenSim.root")

# Show progress of processing.
main.add_module("Progress")

basf2.print_path(main)

basf2.process(main)
