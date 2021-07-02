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
    environmentType="default",
    filename=b2.find_file(f"starterkit/2021/1111540100_eph3_BGx0_{filenumber}.root", "examples"),
    path=main,
)

# Add the database with the classifier weight files for the FEI
b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# Get FEI default channels.
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels(
    chargedB=True,
    neutralB=False,
    hadronic=True,
    semileptonic=False,
    baryonic=True,
)

# Set up FEI configuration specifying the FEI prefix
configuration = fei.FeiConfiguration(
    prefix="FEIv4_2020_MC13_release_04_01_01", monitor=False
)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
main.add_path(feistate.path)

# Add MC matching when applying to MC.
# This is required for variables like isSignal and mcErrors below
ma.matchMCTruth(list_name="B+:generic", path=main)

# Rank B+ candidates by signal classifier output
ma.rankByHighest(
    particleList="B+:generic",
    variable="extraInfo(SignalProbability)",
    outputVariable="FEIProbabilityRank",
    path=main,
)
vm.addAlias("FEIProbRank", "extraInfo(FEIProbabilityRank)")

vm.addAlias("SigProb", "extraInfo(SignalProbability)")
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

# Process events
b2.process(main)
print(b2.statistics)
