#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma
import stdV0s

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
ma.inputMdstList(
    filelist=[b2.find_file(f"starterkit/2021/1111540100_eph3_BGx0_{filenumber}.root", "examples")],
    path=main,
)

# fill final state particle lists
ma.fillParticleList(
    "e+:uncorrected",
    "electronID > 0.1 and dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance",
    path=main,
)
stdV0s.stdKshorts(path=main)

# combine final state particles to form composite particles
ma.reconstructDecay(
    "J/psi:ee -> e+:uncorrected e-:uncorrected", cut="abs(dM) < 0.11", path=main
)

# combine J/psi and KS candidates to form B0 candidates
ma.reconstructDecay(
    "B0 -> J/psi:ee K_S0:merged",
    cut="",
    path=main,
)

# match reconstructed with MC particles
ma.matchMCTruth("B0", path=main)

# save variables to an output file (ntuple)
ma.variablesToNtuple(
    "B0",
    variables=['Mbc', 'isSignal'],
    filename="Bd2JpsiKS.root",
    treename="tree",
    path=main,
)

# start the event loop (actually start processing things)
b2.process(main)

# print out the summary
print(b2.statistics)
