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
    environmentType="default",
    filelist=[b2.find_file(f"starterkit/2021/1111540100_eph3_BGx0_{filenumber}.root", "examples")],
    path=main,
)

# fill final state particle lists
ma.fillParticleList("e+:uncorrected", "", path=main)
stdV0s.stdKshorts(path=main)

# Start the event loop (actually start processing things)
b2.process(main)

# print out the summary
print(b2.statistics)
