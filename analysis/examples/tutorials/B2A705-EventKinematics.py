#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to use the Event Kinematics framework
# usage : basf2 -i myMDSTFile.root B2A705-EventKinematics.py
#
# Contributors: Frank Meier (March 2020)
# frank.meier@desy.de
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

# calculate the event kinematics variables using the most likely mass hypothesis for each track and applying the predefined
# selection criteria for tracks (pt > 0.1 and thetaInCDCAcceptance and abs(dz) < 3 and dr < 0.5) and for photons (E > 0.05 and
# thetaInCDCAcceptance)
ma.buildEventKinematics(default_cleanup=True, fillWithMostLikely=True, path=my_path)

# The predefined collection 'event_kinematics' contains all variables that require the Event Kinematics module to be run. Those are
# the total missing momentum of the event as well as its x-, y-, and z-component, both in the lab and in the CMS frame. Furthermore,
# the variable collection contains the missing energy of the event in the CMS frame, the missing mass squared of the event, the
# total visible energy of the event in the CMS frame and the total energy of all photons in the event.
ma.variablesToNtuple('', variables=['event_kinematics'], filename='B2A705-EventKinematics.root', path=my_path)

# Process the events
b2.process(my_path)
# print out the summary
print(b2.statistics)
