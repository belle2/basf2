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
# This tutorial demonstrates how to apply KsSelector to existing         #
# particle list.                                                         #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import stdV0s as stdV0s
import ksSelector as ksSelector

output_file = 'output.root'

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B02JpsiKs_Jpsi2mumu_Ks2pipi.root', 'examples', False),
             path=my_path)

# load V0s and copy it
stdV0s.stdKshorts(path=my_path)

# load globaltag
useCentralDB = True
centralDB = 'KsFinder_dev'
localDB = ''
if useCentralDB:
    b2.conditions.prepend_globaltag(centralDB)
else:
    b2.conditions.append_testing_payloads(localDB)

# apply ksSelector to the K_S0 particle list
# list type can be 'all', 'standard', 'tight', or 'loose'
# If 'all', no cut is applied by KsSelector, and mva output become available via extraInfo.
# if other list type, cut is applied on the original particle list.
ksSelector.ksSelector(particleListName='K_S0:merged',
                      identifier_Ks="sugiura_KsFinder_V0Selector",
                      identifier_vLambda="sugiura_KsFinder_LambdaVeto",
                      output_label_name='standard',
                      extraInfoName_V0Selector='V0_mva',
                      extraInfoName_LambdaVeto='Lam_mva',
                      path=my_path)

ma.matchMCTruth(list_name='K_S0:standard', path=my_path)

# set variables
vars = vc.kinematics + vc.mc_kinematics + vc.mc_truth
vars += ['extraInfo(V0_mva)']
vars += ['extraInfo(Lam_mva)']

# output
ma.variablesToNtuple('K_S0:standard',
                     variables=vars,
                     filename=output_file,
                     treename='tree',
                     path=my_path)

b2.process(my_path)

# print out the summary
print(b2.statistics)
