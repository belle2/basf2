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
import mdst

parser = argparse.ArgumentParser(description="Generates nbar events and save TOP timing fits to neutral clusters")
parser.add_argument('--mom', default=1, help='Momentum of nbars to generate [GeV/c]')
parser.add_argument('--saveFits', action='store_true', default=False, help='Flag to save plots of RooFits')
parser.add_argument('--minClusterE', type=float, default=0.5, help='Minimum (incl.) clusterE to be considered for timing')
parser.add_argument('--minNphotons', type=int, default=2, help='Minimum (incl.) no. of Cherenkov photons for fit')
parser.add_argument('--minClusterNHits', type=float, default=1, help='Minimum (incl.) no. of crystals in cluster required')
parser.add_argument('--includeSlotsWithTracks', action='store_true', default=False, help='Flag to save plots of RooFits')
parser.add_argument('--saveMoreFitParams', action='store_true', default=False,
                    help='Development flag to save more RooFit params (e.g. RooFit errors, fit params)')

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
    phiParams=[11.25],  # initial momentum towards slot1
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
path.add_module("TOPBackSplashTiming",
                saveFits=args.saveFits,
                minClusterE=args.minClusterE,
                minNphotons=args.minNphotons,
                minClusterNHits=args.minClusterNHits,
                includeSlotsWithTracks=args.includeSlotsWithTracks,
                saveMoreFitParams=args.saveMoreFitParams,
                )

# Save mdst with timing, no. of fitted photons and chi-2/dof
mdst.add_mdst_output(path, mc=True, additionalBranches=['TOPBackSplashFitResults'])

path.add_module("Progress")

b2.process(path, calculateStatistics=True)

# show call statistics
print(b2.statistics)
