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
# KsSelector.                                                            #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import stdV0s as stdV0s
import ksSelector as ksSelector

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('ccbar_background.root', 'examples', False),
             path=my_path)

# load K_S0 particle list
stdV0s.stdKshorts(path=my_path)
ma.matchMCTruth(list_name='K_S0:merged', path=my_path)

# apply cuts suitable for V0Selector
# remove Lambda from particle list
ma.cutAndCopyLists('K_S0:V0Selector', 'K_S0:merged', cut='mcPDG!=3122', path=my_path)
# apply cuts suitable for LambdaVeto
# only include long lived particle, K_S0 and Lambda
ma.cutAndCopyLists('K_S0:LambdaVeto', 'K_S0:merged', cut='isSignal==1 or abs(mcPDG)==3122', path=my_path)

# add variable aliases required for KsSelector training
ksSelector.add_variable_collection()

# add variables required for KsSelector training
vars = ['ks_selector_info', 'isNotContinuumEvent', 'isSignal']

# output
ma.variablesToNtuple('K_S0:V0Selector',
                     variables=vars,
                     filename='KsSelector_train_V0Selector.root',
                     treename='tree',
                     path=my_path)

ma.variablesToNtuple('K_S0:LambdaVeto',
                     variables=vars,
                     filename='KsSelector_train_LambdaVeto.root',
                     treename='tree',
                     path=my_path)

# Process the events
b2.process(my_path, max_event=10000)

# print out the summary
print(b2.statistics)
