#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to use the Event Shape framework        #
# usage : basf2 -i myMDSTFile.root B2A704-EventShape.py                  #
#                                                                        #
##########################################################################


import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)


# Creates a list of good tracks (using the pion mass hypothesis)
# and good gammas with very minimal cuts
ma.fillParticleList(decayString='pi+:goodtracks',
                    cut='pt> 0.1',
                    path=my_path)
ma.fillParticleList(decayString='gamma:minimal',
                    cut='E > 0.1',
                    path=my_path)

# Builds the event shape enabling explicitly ALL the variables.
# Most of them are actually enabled by default, but here we prefer
# to list explicitly all the flags
ma.buildEventShape(inputListNames=['pi+:goodtracks', 'gamma:minimal'],
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

# Here we use the predefined collection 'event_shape', that contains thrust,
# sphericity, aplanarity, FW ratios up to 4, harmonic moments w/respect to
# the thrust axis up to 4 and all the cleo cones w/respect to the thrust
# axis. In addition, we will save also the forward and backward hemisphere (
# or "jet") energies, and the 2nd order harmonic moment calculated with
# respect to the collision axis (i.e. the z axis)
ma.variablesToNtuple(
    '',
    variables=[
        *vc.event_shape,  # [1] see below
        'backwardHemisphereEnergy',
        'forwardHemisphereEnergy',
        'harmonicMoment(2, collision)'
    ],
    filename='B2A704-EventShape.root',
    path=my_path
)

# [1] Note: The * operator "unpacks" the list of variables provided by the
# variable collection (because we don't want to get a list in a list, but just
# add the elements): ```[*[1, 2, 3], 4] == [1, 2, 3, 4])```

# Process the events
b2.process(my_path)
# print out the summary
print(b2.statistics)
