#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to print out the
# contents of the DataStore for each event.
# This is useful for debugging purposes, e.g. to check if
# the MC files contain the neccessary (MDST) StoreArrays.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import printDataStore
from modularAnalysis import analysis_main

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-gsim-BKGx0.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-gsim-BKGx0.root) does not exist. '
             'Please run B2A101-Y4SEventGeneration.py and B2A103-SimulateAndReconstruct-withoutBeamBkg.py '
             'tutorial scripts first.')

# load input ROOT file
inputMdst('default', 'B2A101-Y4SEventGeneration-gsim-BKGx0.root')

# print out the contents of the DataStore
printDataStore()

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
