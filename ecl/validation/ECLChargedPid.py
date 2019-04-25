#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
<contact>Marco Milesi, marco.milesi@unimelb.edu.au</contact>
<description> This steering script generates 1000 particle gun events for a charged stable particle,
runs the fullsim and reconstruction, and dumps ECL PID validation info in an ntuple.</description>
</header>
"""

import os
import glob
import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction

# Pdg code of the charged stable particle (can be +/-)
pdgId = 11

# Create paths
main = basf2.create_path()

# Event setting and info
eventinfosetter = basf2.register_module("EventInfoSetter")
eventinfosetter.param({"evtNumList": [1000], "runList": [1]})
main.add_module(eventinfosetter)

# Fixed random seed.
basf2.set_random_seed("P1s@Merd@")

# Single particle generator settings.
pGun = basf2.register_module("ParticleGun")
pGun.set_name(f"ParticleGun_{pdgId}")
param_pGun = {
    "pdgCodes": [pdgId],
    "nTracks": 1,
    "momentumGeneration": "uniform",
    "momentumParams": [0.2, 5.0],
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

if "BELLE2_BACKGROUND_DIR" in os.environ:
    bg = glob.glob(os.environ["BELLE2_BACKGROUND_DIR"] + "/*.root")
else:
    basf2.B2WARNING("Env variable BELLE2_BACKGROUND_DIR is not set.")
    bg = None

add_simulation(main, bkgfiles=bg)
add_reconstruction(main)

validation_module = basf2.register_module("ECLChargedPIDDataAnalysisValidation")
validation_module.set_name(f"ECLChargedPIDDataAnalysisValidation_{pdgId}")
validation_module.param("outputFileName", "ECLChargedPid")
validation_module.param("inputPdgId", pdgId)
main.add_module(validation_module)

basf2.process(main)
