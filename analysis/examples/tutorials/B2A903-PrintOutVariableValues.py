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
from modularAnalysis import fillParticleListFromMC
from modularAnalysis import printVariableValues
from modularAnalysis import analysis_main

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-evtgen.root) does not exist. Please run B2A101-Y4SEventGeneration.py tutorial script first.'
             )

# load input ROOT file
inputMdst('B2A101-Y4SEventGeneration-evtgen.root')

# create charged kaon and pion particle lists with names "K-:gen" and "pi-:gen"
# all generated kaons(pions) will be added to the kaon(pion) list
fillParticleListFromMC('K-:gen', '')
fillParticleListFromMC('pi-:gen', '')

# print charge, energy and total momentum of generated kaons
# and x,y,z components of momenta for generated pions
#
# the list of all available variables can be obtained by executing
# basf2 analysis/scripts/variables.py
printVariableValues('K-:gen', ['charge', 'E', 'p'])
printVariableValues('pi-:gen', ['charge', 'px', 'py', 'pz'])

# Process the events
process(analysis_main)

# print out the summary
print statistics
