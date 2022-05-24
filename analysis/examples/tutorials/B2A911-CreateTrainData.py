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
# This tutorial demonstrates how to create data sample for  train weight file for KsSelector.    #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import stdV0s as stdV0s
import ksSelector as ksSelector

output_file = "KsSelector_train.root"

# create path
my_path = b2.create_path()

# input file name
filelistSIG = ['']

# load input ROOT file
ma.inputMdstList(environmentType='default',
                 filelist=filelistSIG,
                 path=my_path)

# load K_S0 particle list
stdV0s.stdKshorts(path=my_path)
ma.applyCuts('K_S0:merged', '0.45 < M < 0.55', path=my_path)
ma.matchMCTruth(list_name='K_S0:merged', path=my_path)

# apply cuts suitable for V0Selector
# remove Lambda from particle list
ma.copyList('K_S0:V0Selector', 'K_S0:merged', path=my_path)
ma.applyCuts('K_S0:V0Selector', 'mcPDG!=3122', path=my_path)
# apply cuts suitable for LambdaVeto
# only include long lived particle, K_S0 and Lambda
ma.copyList('K_S0:LambdaVeto', 'K_S0:merged', path=my_path)
ma.applyCuts('K_S0:LambdaVeto', 'isSignal==1 or abs(mcPDG)==3122', path=my_path)

# add variable aliases required for KsSelector training
ksSelector.add_variable_collection()

# add variables required for KsSelector training
vars = ['ks_selector_info', 'isNotContinuumEvent', 'isSignal']

# output
ma.variablesToNtuple('K_S0:V0Selector',
                     variables=vars,
                     filename=output_file,
                     treename='V0Selector',
                     path=my_path)

ma.variablesToNtuple('K_S0:LambdaVeto',
                     variables=vars,
                     filename=output_file,
                     treename='LambdaVeto',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
