#!/usr/bin/env python3

import sys
import basf2 as b2
import fei
import modularAnalysis as ma
from variables import variables as vm

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
ma.inputMdst(
    filename=b2.find_file(f"starterkit/2021/1111540100_eph3_BGx0_{filenumber}.root", "examples"),
    path=main,
)

# Add the database with the classifier weight files for the FEI
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Get FEI default channels.  [S10]
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels(
    chargedB=True,
    neutralB=False,
    hadronic=True,
    semileptonic=False,
    baryonic=True,
)  # [E10]
# Set up FEI configuration specifying the FEI prefix [S20]
configuration = fei.FeiConfiguration(
    prefix="FEIv4_2021_MC14_release_05_01_12", monitor=False
)  # [E20]

# Get FEI path [S30]
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
main.add_path(feistate.path)  # [E30]

# Add MC matching when applying to MC. [S40]
# This is required for variables like isSignal and mcErrors below
ma.matchMCTruth(list_name="B+:generic", path=main)  # [E40]

# Rank B+ candidates by signal classifier output [S50]
ma.rankByHighest(
    particleList="B+:generic",
    variable="extraInfo(SignalProbability)",
    outputVariable="FEIProbabilityRank",
    path=main,
)
vm.addAlias("FEIProbRank", "extraInfo(FEIProbabilityRank)")

vm.addAlias("SigProb", "extraInfo(SignalProbability)")  # [S41]
vm.addAlias("decayModeID", "extraInfo(decayModeID)")

# Store tag-side variables of interest.
ma.variablesToNtuple(
    "B+:generic",
    [
        "Mbc",
        "deltaE",
        "mcErrors",
        "SigProb",
        "decayModeID",
        "FEIProbRank",
        "isSignal",
    ],
    filename="B_charged_hadronic.root",
    path=main,
)
#  [E41|E50]
# Process events
b2.process(main)
print(b2.statistics)
