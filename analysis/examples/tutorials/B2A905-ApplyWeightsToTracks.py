#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to weight tracks using
# LooKUpTable from the database
#
# Contributors: I. Komarov (April 2018)
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

# ID of weight table is taked from B2A904
weight_table_id = "ParticleReweighting:TestMomentum"

# We know what weight info will be added (see B2A904),
# so we add aliases and add it ot tools
variables.addAlias('Weight', 'extraInfo(' + weight_table_id + '_Weight)')
variables.addAlias('StatErr', 'extraInfo(' + weight_table_id + '_StatErr)')
variables.addAlias('SystErr', 'extraInfo(' + weight_table_id + '_SystErr)')
variables.addAlias('binID', 'extraInfo(' + weight_table_id + '_binID)')
toolsPi = ['CustomFloats[p:pz:Weight:StatErr:SystErr:binID]', '^pi+:gen']


# We configure weighing module
reweighter = register_module('ParticleWeighting')
reweighter.param('tableName', weight_table_id)
reweighter.param('particleList', 'pi+:gen')
analysis_main.add_module(reweighter)


# write out the flat ntuple
ntupleFile('B2A905-ApplyWeightsToTracks.root')
ntupleTree('dsttree', 'pi+:gen', toolsPi)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
