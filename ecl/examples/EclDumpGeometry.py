#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################################################
#
# Write out the location and direction of all ECL crystals. Uses information
# from the payload ECLCrystalsPositionAndShape for the experiment and run of
# the input file. Information from inputFile.root is not otherwise used.
#
# basf2 -n 1 [-i "inputFile.root"] EclDumpGeometry.py
#
# (c) Christopher Hearty, 2020 (hearty@physics.ubc.ca)
#
################################################################################

import basf2 as b2
from modularAnalysis import inputMdst
from basf2 import conditions

# ..path
mypath = b2.create_path()

# ..Read ecl geometry from localdb by adding a line such as:
#  conditions.prepend_testing_payloads("localdb/database.txt")
#  where localdb contains the ECLCrystalsPositionAndShape payload
print(b2.conditions.globaltags)
inputMdst(
    'default',
    '/group/belle2/dataprod/Data/OfficialReco/proc11/e0010/4S/GoodRuns/r05902/skim/hlt_mumu_2trk/mdst/sub00/*.root',
    path=mypath)

# ..ECL geometry
mypath.add_module('Gearbox')
mypath.add_module('Geometry')
mypath.add_module('ECLDumpGeometry')

# Process the events
b2.process(mypath)
print(b2.statistics)
