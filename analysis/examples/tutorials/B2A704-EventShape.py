#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to use the Event Shape framework
# usage : basf2 -i myMDSTFile.root B2A704-EventShape.py
#
# Contributors: Umberto Tamponi (October 2018)
# tamponi@to.infn.it
#
####################################################################


from basf2 import *
from modularAnalysis import *
import variableCollections

# load input ROOT file. Please set it using the -i option
filelistSIG = ''
inputMdst('default', '')

# Creates a list of all the good tracks (using the pion mass hypothesys)
# and good gammas with very minimal cuts
fillParticleList('pi+:all', 'pt> 0.1')
fillParticleList('gamma:all', 'E > 0.1')

# Builds the event shape enabling explicitly ALL the variables.
# Most of the are actually enabled by defoult, but here we prefer
# to list explicitly all the flags
buildEventShape(inputListNames=['pi+:all', 'gamma:all'],
                allMoments=True,
                foxWolfram=True,
                harmonicMoments=True,
                cleoCones=True,
                thrust=True,
                collisionAxis=True,
                jets=True,
                sphericity=True,
                checkForDuplicates=False)

# Here we use the pre-defined collection 'EventShape', that contains
# thrust, sphericity, aplanarity, FW ratios up to 4, harmonic moments w/respect to
# the thrust axis up to 4 and all the cleo cones w/respect to the thrust axis.
# In addition, we will save also the forward and backward hemisphere (or "jet") energies,
# and the 2nd order harmonic moment calculate respect to the collision axis (i.e. the z axis)
variablesToNtuple('',
                  variables=['EventShape',
                             'backwardHemisphereEnergy',
                             'forwardHemisphereEnergy',
                             'harmonicMoment(2, collision)'],
                  filename='B2A704-EventShape.root')


# Process the events
process(analysis_main)
# print out the summary
print(statistics)
