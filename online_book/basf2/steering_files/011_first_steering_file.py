#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
filedirectory = "/group/belle2/users/tenchini/prerelease-05-00-00a/1111540100/"
ma.inputMdstList(
    environmentType="default",
    filelist=[f"{filedirectory}/1111540100_eph3_BGx0_{filenumber}.root"],
    path=main,
)

# Start the event loop (actually start processing things)
b2.process(main)