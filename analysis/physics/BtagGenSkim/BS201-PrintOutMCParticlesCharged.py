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

# load input ROOT file. First you must produce these files!
inputMdst('BS101-BBChargedGen.root')

# print out the contents of the StoreArray<MCParticle>
printPrimaryMCParticles()

# Process the events
process(analysis_main)

# print out the summary
print statistics
