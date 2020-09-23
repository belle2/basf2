#!/usr/bin/env python3

import sys
import basf2 as b2
import modularAnalysis as ma
import fei


# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()


# Add the database with the classifier weight files for the FEI
# You can use the command b2conditionsdb-recommend
b2.conditions.globaltags = ["analysis_tools_release-04-02"]


ma.fillParticleList('mu+', "muonID > 0.9 and dr < 0.5 and abs(dz) < 2 and p > 0.5 and thetaInCDCAcceptance", path=main)
ma.reconstructDecay('B+:signal -> mu+', '', path=main)
ma.reconstructDecay('Upsilon(4S) -> B-:generic B+:signal', '', path=main)


# Process events
b2.process(main)
print(b2.statistics)
