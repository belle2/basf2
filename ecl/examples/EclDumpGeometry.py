#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

################################################################################
#
# Write out the location and direction of all ECL crystals. Uses information
# from the payload ECLCrystalsPositionAndShape for the experiment and run of
# the input file. Information from inputFile.root is not otherwise used.
#
# basf2 -n 1 [-i "inputFile.root"] EclDumpGeometry.py [newGeom]
#
# newGeom = 1 will read it from global tag ecl_alignment
# newGeom = 2 reads the payload from localdb;
# If not specified, the payload will be taken from the global tag of "inputFile.root"
#
################################################################################

import sys
import basf2 as b2
from modularAnalysis import inputMdst

# ..path
mypath = b2.create_path()

# ..override ECL geometry if requested
newGeom = 0
narg = len(sys.argv)
if(narg == 2):
    newGeom = int(sys.argv[1])
if(newGeom == 1):
    b2.conditions.globaltags = ['ecl_alignment']
if(newGeom == 2):
    b2.conditions.prepend_testing_payloads("localdb/database.txt")
print(b2.conditions.globaltags)
inputMdst(
    '/group/belle2/dataprod/Data/OfficialReco/proc11/e0010/4S/GoodRuns/r05902/skim/hlt_mumu_2trk/mdst/sub00/*.root',
    path=mypath)

# ..ECL geometry
mypath.add_module('Gearbox')
mypath.add_module('Geometry')
mypath.add_module('ECLDumpGeometry')

# ..Process the events
b2.process(mypath)
print(b2.statistics)
