#!/usr/bin/env python3

import basf2 as b2
import modularAnalysis as ma

# create path
main = b2.Path()

# load input data from mdst/udst file
ma.inputMdstList(
    environmentType="default",
    filelist=["/group/belle2/users/tenchini/prerelease-05-00-00a/1111540100/1111540100_eph3_BGx0_99.root"],
    path=main
)

# Start the event loop (actually start processing things)
b2.process(main)
