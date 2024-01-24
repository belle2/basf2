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
# This tutorial demonstrates how to get photon detection efficiency      #
# ratio variables added to extraInfo for particles in photon lists.      #
#                                                                        #
##########################################################################


import basf2 as b2
import modularAnalysis as ma
import variables as va

# create path
path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=path)

# ID of weight table
weight_table_id = "PhotonEfficiencyDataMCRatio_proc13MC15_November2022"

b2.conditions.prepend_globaltag(ma.getAnalysisGlobaltag())

# We know what weight info will be added,
# so we add aliases and add it to the tools
va.variables.addAlias('Weight', f'extraInfo({weight_table_id}_Weight)')
va.variables.addAlias('StatErrUp', f'extraInfo({weight_table_id}_StatErrUp)')
va.variables.addAlias('StatErrDown', f'extraInfo({weight_table_id}_StatErrDown)')
va.variables.addAlias('SystErrUp', f'extraInfo({weight_table_id}_SystErrUp)')
va.variables.addAlias('SystErrDown', f'extraInfo({weight_table_id}_SystErrDown)')
va.variables.addAlias('TotalErrUp', f'extraInfo({weight_table_id}_TotalErrUp)')
va.variables.addAlias('TotalErrDown', f'extraInfo({weight_table_id}_TotalErrDown)')
# Just take any photons for example purposes
simple_selection = 'theta > 0.296706 and theta < 2.6179'
ma.fillParticleList(decayString='gamma:full',
                    cut=simple_selection,
                    path=path)

# Apply the photon module
ma.addPhotonEfficiencyRatioVariables(['gamma:full'], weight_table_id, path=path)

# The ratios are added wrt to E, theta, phi of the photon
weight_vars = ['p', 'E', 'theta', 'phi',
               'Weight', 'TotalErrUp', 'TotalErrDown', 'StatErrUp', 'StatErrDown', 'SystErrUp', 'SystErrDown',
               ]


ma.variablesToNtuple(decayString='gamma:full',
                     variables=weight_vars,
                     filename='B2A907-PhotonDetectionEfficiencyRatios.root',
                     path=path)

# process the events
b2.process(path)

# print out the summary
print(b2.statistics)
