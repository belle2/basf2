#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to perform Vertex fits
# using KFit. The following  decay chain (and c.c. decay
# chain):
#
# D*+ -> D0 pi+
#        |
#        +-> K- pi+
#
# is reconstructed and the D0 and D*+ decay vertices are
# fitted.
#
# Note: This example is build upon
# B2A301-Dstar2D0Pi-Reconstruction.py
#
# Note: This example uses the signal MC sample created with
# release-01-00-00
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (December 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from modularAnalysis import vertexKFit
from stdCharged import *

# check if the required input file exists (from B2A101 example)
import os.path
import sys
if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    sys.exit('Required input file (B2A101-Y4SEventGeneration-evtgen.root) does not exist. '
             'Please run B2A101-Y4SEventGeneration.py tutorial script first.')

# load input ROOT file
inputMdst('default', 'B2A101-Y4SEventGeneration-evtgen.root')

# use standard final state particle lists
# creates "pi+:all" ParticleList (and c.c.)
fillParticleListFromMC('pi+:gen', '')

variables.addAlias('Weight', 'extraInfo(Weight)')
variables.addAlias('StatErr', 'extraInfo(StatErr)')
variables.addAlias('SystErr', 'extraInfo(SystErr)')

reweighter = register_module('ParticleWeighting')
reweighter.param('tableName', "ParticleReweighting:TestMomentum")
reweighter.param('particleList', 'pi+:gen')
analysis_main.add_module(reweighter)

toolsPi = ['CustomFloats[p:pz:Weight:StatErr:SystErr]', '^pi+:gen']

# write out the flat ntuple
ntupleFile('B2A905-ApplyWeightsToTracks.root')
ntupleTree('dsttree', 'pi+:gen', toolsPi)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
