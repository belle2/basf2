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
# This tutorial demonstrates how to use the Event Kinematics framework   #
# usage : basf2 -i myMDSTFile.root B2A705-EventKinematics.py             #
#                                                                        #
##########################################################################


import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)

# calculate the event kinematics variables using the most likely mass
# hypothesis for each track and applying the predefined selection criteria
# for tracks (pt > 0.1 and thetaInCDCAcceptance and abs(dz) < 3 and dr < 0.5)
# and for photons (E > 0.05 and thetaInCDCAcceptance)
ma.buildEventKinematics(default_cleanup=True, fillWithMostLikely=True, path=my_path)

# The event kinematic variables can also be calculated based on the generated
# particles:
ma.buildEventKinematicsFromMC(path=my_path)

# The predefined collection 'event_kinematics' contains all variables that
# require the Event Kinematics module to be run. Those are the total missing
# momentum of the event as well as its x-, y-, and z-component, both in the
# lab and in the CMS frame. Furthermore, the variable collection contains the
# missing energy of the event in the CMS frame, the missing mass squared of
# the event, the total visible energy of the event in the CMS frame and the
# total energy of all photons in the event. Similarly, the MC version of this
# collection 'mc_event_kinematics' contains the missing mass squared, missing
# energy and missing momentum based on the MC particles.
ma.variablesToNtuple('', [*vc.event_kinematics, *vc.mc_event_kinematics], filename='B2A705-EventKinematics.root', path=my_path)

# Process the events
b2.process(my_path)
# print out the summary
print(b2.statistics)
