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
# This tutorial demonstrates how to create data sample for training of   #
# LambdaSelector.                                                        #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import stdV0s as stdV0s
import variables.collections as vc

import LambdaSelector as LambdaSelector

# Creat analysis path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('ccbar_background.root', 'examples', False), path=my_path)
# load Lambda0 particle list
stdV0s.stdLambdas(path=my_path)
ma.matchMCTruth(list_name='Lambda0:merged', path=my_path)

# apply cuts suitable for V0Selector
# remove KS0 from particle list
ma.cutAndCopyLists('Lambda0:V0Selector', 'Lambda0:merged', cut='mcPDG!=310', path=my_path)
# apply cuts suitable for KsVeto
# only include long lived particle, K_S0 and Lambda
ma.cutAndCopyLists('Lambda0:KsVeto',     'Lambda0:merged', cut='isSignal==1 or abs(mcPDG)==310', path=my_path)

# add variable aliases required for Lambda0Selector training
LambdaSelector.add_variable_collection()

# add variables required for Lambda0Selector training
vars = ['lambda_selector_info', 'isNotContinuumEvent', 'isSignal', 'nMCDaughters', 'mcErrors', 'mcPDG',
        'ArmenterosLongitudinalMomentumAsymmetry', 'pt', 'p', 'InvM', 'M', 'V0Deltad0', 'V0Deltaz0']
vars += vc.kinematics + vc.mc_kinematics + vc.mc_truth + vc.mc_flight_info

# output
ma.variablesToNtuple('Lambda0:V0Selector',
                     variables=vars,
                     filename='Lambda0Selector_train_V0Selector.root',
                     treename='tree',
                     path=my_path)

ma.variablesToNtuple('Lambda0:KsVeto',
                     variables=vars,
                     filename='Lambda0Selector_train_KsVeto.root',
                     treename='tree',
                     path=my_path)

# Process the events
b2.process(my_path, max_event=10000)

# print out the summary
print(b2.statistics)
