#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#               I. Komarov (December 2017)
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B02D0pi0_D02pi0pi0.root', 'examples', False),
             path=my_path)


# use standard final state particle lists
#
# creates "pi0:eff40_May2020Fit" ParticleList
# see Standard Particles section at https://software.belle2.org/
stdPi0s(listtype='eff40_May2020Fit', path=my_path)

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
ma.reconstructDecay(decayString='D0:pi0pi0 -> pi0:eff40_May2020Fit pi0:eff40_May2020Fit',
                    cut='1.7 < M < 2.0',
                    path=my_path)

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
ma.reconstructDecay(decayString='B0:all -> D0:pi0pi0 pi0:eff40_May2020Fit',
                    cut='5.24 < Mbc < 5.29 and abs(deltaE) < 1.0',
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='B0:all',
                path=my_path)


# Select variables that we want to store to ntuple
B0_vars = vc.inv_mass + \
    vc.mc_truth + \
    vu.create_aliases_for_selected(
        list_of_variables=vc.inv_mass + vc.mc_truth,
        decay_string='B0 -> ^D0 ^pi0') + \
    vu.create_aliases_for_selected(
        list_of_variables=vc.cluster,
        decay_string='B0 -> D0 [pi0 -> ^gamma ^gamma]')

pi0_vars = vc.mc_truth + \
    vc.kinematics + \
    ['extraInfo(BDT)', 'decayAngle(0)', 'weightedAverageECLTime'] + \
    vu.create_aliases_for_selected(
        list_of_variables=vc.cluster + vc.kinematics,
        decay_string='pi0 -> ^gamma ^gamma')

# Saving variables to ntuple
output_file = 'B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.root'
ma.variablesToNtuple('B0:all', B0_vars,
                     filename=output_file,
                     treename='b0',
                     path=my_path)
ma.variablesToNtuple('pi0:eff40_May2020Fit', pi0_vars,
                     filename=output_file,
                     treename='pi0',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
