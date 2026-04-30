#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------
# Example steering script that generates B0B0bar events with BGx1 overlay
# and applies the TOPBackSplashTiming module
# ---------------------------------------------------------------------------------

import basf2 as b2
import simulation as si
import reconstruction as re
from generators import add_evtgen_generator
import glob
import argparse

parser = argparse.ArgumentParser(description="Generates BB events and save TOP timing fits to neutral clusters")
parser.add_argument('--saveFits', action='store_true', default=False, help='Flag to save plots of RooFits')
parser.add_argument('--minClusterE', type=float, default=0.5, help='Minimum (incl.) clusterE to be considered for timing')
parser.add_argument('--minNphotons', type=int, default=2, help='Minimum (incl.) no. of Cherenkov photons for fit')
parser.add_argument('--minClusterNHits', type=float, default=1, help='Minimum (incl.) no. of crystals in cluster required')
parser.add_argument('--includeSlotsWithTracks', action='store_true', default=False, help='Flag to save plots of RooFits')
parser.add_argument('--saveMoreFitParams', action='store_true', default=False,
                    help='Development flag to save more RooFit params (e.g. RooFit errors, fit params)')

args = parser.parse_args()

# bg = glob.glob('/group/belle2/dataprod/BGOverlay/run2/prerelease-08-00-00a/new_overlay/BGx1/set2/BGforOverlay-6*.root')
bg = glob.glob('/group/belle2/dataprod/BGOverlay/run2/prerelease-08-00-00a/new_overlay/BGx1/set?/*.root')

path = b2.create_path()

path.add_module("EventInfoSetter", evtNumList=1000, expList=1004)
path.add_module("EventInfoPrinter")

# EvtGen
add_evtgen_generator(path=path, finalstate='mixed')

# detector and L1 trigger simulation and reco
si.add_simulation(path=path,  bkgfiles=bg)
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
path.add_module("RootOutput",
                outputFileName="output_TOPBackSplashTiming.root",
                additionalBranchNames=['TOPBackSplashFitResult']
                )

# run
path.add_module("Progress")


# generate events
b2.process(path, calculateStatistics=True)

# show call statistics
print(b2.statistics)
