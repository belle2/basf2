#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
#                                                                        #
# Stuck? Ask for help at questions.belle2.org                            #
#                                                                        #
# This tutorial demonstrates how to weight tracks using                  #
# LooKUpTable from the database                                          #
#                                                                        #
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
ma.inputMdst(filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)

# load V0s and copy it
stdV0s.stdKshorts(path=my_path)
ma.copyList('K_S0:standard', 'K_S0:merged', path=my_path)
ma.matchMCTruth(list_name='K_S0:standard', path=my_path)

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
ksSelector.ksSelector(particleListName='K_S0:standard',
                      identifier_Ks="sugiura_KsFinder_V0Selector",
                      identifier_vLambda="sugiura_KsFinder_LambdaVeto",
                      listtype='all',
                      extraInfoName_V0Selector='V0_mva',
                      extraInfoName_LambdaVeto='Lam_mva',
                      path=my_path)

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
