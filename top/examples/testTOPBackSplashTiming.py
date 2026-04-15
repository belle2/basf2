#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------
# Example steering script that generates anti-neutron events with fixed momentum
# and applies the TOPBackSplashTiming module
# ---------------------------------------------------------------------------------

import basf2 as b2
import simulation as si
import reconstruction as re
import argparse

parser = argparse.ArgumentParser(description="Choose anti-neutron momentum")
parser.add_argument('--mom', default=1, help='')
args = parser.parse_args()

path = b2.create_path()

path.add_module("EventInfoSetter", evtNumList=100)
path.add_module("EventInfoPrinter")

# control muon - needed to set event T0
path.add_module(
    "ParticleGun",
    pdgCodes=[13],
    nTracks=0,
    momentumGeneration="fixed",
    momentumParams=[1],
    thetaGeneration="fixed",
    thetaParams=[92],
    phiGeneration="fixed",
    phiParams=[11.25],
    vertexGeneration="fixed",
    xVertexParams=[0],
    yVertexParams=[0],
    zVertexParams=[0],
    independentVertices=False,
).set_name("MuonGun")

# nbar
path.add_module(
    "ParticleGun",
    pdgCodes=[-2112],  # anti-n0
    nTracks=0,
    momentumGeneration="fixed",
    momentumParams=[float(args.mom)],
    thetaGeneration="uniform",
    thetaParams=[0, 180],
    phiGeneration="uniform",
    phiParams=[0, 360],
    vertexGeneration="fixed",
    xVertexParams=[0],
    yVertexParams=[0],
    zVertexParams=[0],
    independentVertices=False,
).set_name("AntiNeutronGun")

# detector and L1 trigger simulation and reco
si.add_simulation(path=path)
re.add_reconstruction(path=path)

# call module and plot timing fits
path.add_module("TOPBackSplashTiming", saveFits=True)

# Save mdst with timing, no. of fitted photons and chi-2/dof
path.add_module("RootOutput",
                outputFileName=f"output_TOPBackSplashTiming_{args.mom}.root",
                )

b2.process(path)
