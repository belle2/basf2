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
# This tutorial demonstrates how to apply LambdaSelector to existing     #
# particle list.                                                         #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import stdV0s as stdV0s
import LambdaSelector as LambdaSelector

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B02JpsiKs_Jpsi2mumu_Ks2pipi.root', 'examples', False),
             path=my_path)

# load stdLambda0s
stdV0s.stdLambdas(path=my_path)

b2.conditions.prepend_globaltag("analysis_tools_light-2503-ceres")

# apply LambdaSelector to the Lambda0 particle list
# the Lambda0 particle list type can be 'all', 'standard', 'tight', or 'loose'
# If 'all', no cut is applied by LambdaSelector, and mva output become available via extraInfo.
# if other list type, cut is applied on the original particle list.
LambdaSelector.LambdaSelector(particleListName='Lambda0:merged',
                              output_label_name='standard',
                              extraInfoName_V0Selector='V0_mva',
                              extraInfoName_KsVeto='Ks_mva',
                              path=my_path)

ma.matchMCTruth(list_name='Lambda0:standard', path=my_path)

# set variables
vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth
vars += ['extraInfo(V0_mva)']
vars += ['extraInfo(Ks_mva)']

# output
ma.variablesToNtuple('Lambda0:standard',
                     variables=vars,
                     filename='test.root',
                     treename='tree',
                     path=my_path)

b2.process(my_path)

# print out the summary
print(b2.statistics)
