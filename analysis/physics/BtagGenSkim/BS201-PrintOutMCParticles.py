#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################                                                                                     #
# This tutorial demonstrates how to print out the
# contents of the StoreArray<MCParticle>.
#
# This is useful for debugging purposes, e.g. to check if
# the MC files contain the desired processes/decays.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import printPrimaryMCParticles
from modularAnalysis import analysis_main
import sys

if len(sys.argv) != 2:
    sys.exit('Must provide enough arguments: [input file name]')

inputName = sys.argv[1]

# load input ROOT file. First you must produce these files!
inputMdst(inputName)

# print out the contents of the StoreArray<MCParticle>
printPrimaryMCParticles()

# Process the events
process(analysis_main)

# print out the summary
print statistics
