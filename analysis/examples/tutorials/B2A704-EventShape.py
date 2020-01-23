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


import basf2 as b2
import modularAnalysis as ma
import variables.collections

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)


# Creates a list of all the good tracks (using the pion mass hypothesys)
# and good gammas with very minimal cuts
ma.fillParticleList(decayString='pi+:all',
                    cut='pt> 0.1',
                    path=my_path)
ma.fillParticleList(decayString='gamma:all',
                    cut='E > 0.1',
                    path=my_path)

# Builds the event shape enabling explicitly ALL the variables.
# Most of the are actually enabled by defoult, but here we prefer
# to list explicitly all the flags
ma.buildEventShape(inputListNames=['pi+:all', 'gamma:all'],
                   allMoments=True,
                   foxWolfram=True,
                   harmonicMoments=True,
                   cleoCones=True,
                   thrust=True,
                   collisionAxis=True,
                   jets=True,
                   sphericity=True,
                   checkForDuplicates=False,
                   path=my_path)

# Here we use the pre-defined collection 'event_shape', that contains
# thrust, sphericity, aplanarity, FW ratios up to 4, harmonic moments w/respect to
# the thrust axis up to 4 and all the cleo cones w/respect to the thrust axis.
# In addition, we will save also the forward and backward hemisphere (or "jet") energies,
# and the 2nd order harmonic moment calculate respect to the collision axis (i.e. the z axis)
ma.variablesToNtuple('',
                     variables=['event_shape',
                                'backwardHemisphereEnergy',
                                'forwardHemisphereEnergy',
                                'harmonicMoment(2, collision)'],
                     filename='B2A704-EventShape.root',
                     path=my_path)


# Process the events
b2.process(my_path)
# print out the summary
print(b2.statistics)
