#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <output>PartGunChargedStableGenSim.root</output>
  <cacheable/>
  <contact>Marco Milesi, marco.milesi@unimelb.edu.au</contact>
  <description> This steering script generates 1000 particle gun events for a set of charged stable particles,
runs the fullsim w/ mixed in background, and dumps full output (*Digits containers) in a file.</description>
</header>
"""

import basf2
from simulation import add_simulation
from background import get_background_files
from ROOT import Belle2

# Pdg code of the charged stable particles & antiparticles.
chargedStableList = []
for idx in range(len(Belle2.Const.chargedStableSet)):
    particle = Belle2.Const.chargedStableSet.at(idx)
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
    "nTracks": 8,  # 0 : generate 1 track per pdgId per event.
    "momentumGeneration": "uniform",
    "momentumParams": [0.05, 5.0],
    "thetaGeneration": "uniform",
    "thetaParams": [0, 180],
    "phiGeneration": "uniform",
    "phiParams": [0, 360],
    "vertexGeneration": "uniform",
    "xVertexParams": [0.0, 0.0],
    "yVertexParams": [0.0, 0.0],
    "zVertexParams": [0.0, 0.0],
}
pGun.param(param_pGun)
main.add_module(pGun)

# Detector simulation + bkg.
add_simulation(main, bkgfiles=get_background_files())

# Memory profile.
main.add_module("Profile")

# Create output of event generation + detector simulation.
main.add_module("RootOutput", outputFileName="../PartGunChargedStableGenSim.root")

# Show progress of processing.
main.add_module("Progress")

basf2.print_path(main)

basf2.process(main)
