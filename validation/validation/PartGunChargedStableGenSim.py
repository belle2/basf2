#!/usr/bin/env python3
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
  <output>PartGunChargedStableGenSim.root</output>
  <cacheable/>
  <contact>Marco Milesi, marco.milesi@unimelb.edu.au</contact>
  <description> This steering script generates 1000 particle gun events for a set of charged stable particles,
runs the fullsim w/ mixed in background, and dumps full output (*Digits containers) in a file.</description>
</header>
"""

# NB. Argument parsing is done *before* any import from the ROOT module, otherwise PyROOT will hijack the command-line options
# in case of clashing option names. Most notably, this would happen with the '-h', '--help' option.
import argparse

parser = argparse.ArgumentParser(
    description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
)
parser.add_argument(
    "--bkg_dir",
    type=str,
    default=None,
    help="The directory containing beam bkg files.\n"
    "If not set, basf2 will search for the 'BELLE2_BACKGROUND_DIR' env variable by default,\n"
    "which is defined on the validation server.",
)
args = parser.parse_args()

from ROOT import Belle2  # noqa
from background import get_background_files  # noqa
from simulation import add_simulation  # noqa
import basf2  # noqa

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
    "nTracks": 0,  # 0 : generate 1 track per pdgId per event.
    "momentumGeneration": "uniform",
    "momentumParams": [0.05, 5.0],
    "thetaGeneration": "uniform",
    "thetaParams": [17, 150],  # Generate tracks within CDC acceptance.
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
add_simulation(main, bkgfiles=get_background_files(folder=args.bkg_dir))

# Memory profile.
main.add_module("Profile")

# Create output of event generation + detector simulation.
main.add_module(
    "RootOutput", outputFileName="../PartGunChargedStableGenSim.root"
)

# Show progress of processing.
main.add_module("Progress")

basf2.print_path(main)

basf2.process(main)
