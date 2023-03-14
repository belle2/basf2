#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
ma.inputMdstList(
    filelist=[b2.find_file(f"starterkit/2021/1111540100_eph3_BGx0_{filenumber}.root", "examples")],
    path=main,
)

# start the event loop (actually start processing things)
b2.process(main)
