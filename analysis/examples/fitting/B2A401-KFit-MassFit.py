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
# This tutorial demonstrates how to perform mass fit with
# the KFit. In this example the following decay chain:
#
# B0 -> D0 pi0
#       |
#       +-> pi0 pi0
#
# is reconstructed. The mass fits is performed on D0
# candidates (as well as pi0 candidates by default).
#
# Note: This example is build upon
# B2A302-B02D0Pi0-D02Pi0Pi0-Reconstruction.py
#
# Note: This example uses the signal MC sample created in
# MC campaign 3.5, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

import basf2 as b2
from modularAnalysis import inputMdst
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from vertex import kFit
from modularAnalysis import variablesToNtuple
from stdPi0s import stdPi0s
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
inputMdst(environmentType='default',
          filename=b2.find_file('B02pi0D0_D2kpi_B2Dstarpi_Dstar2Dpi_D2kpi.root', 'examples', False),
          path=my_path)

# use standard final state particle lists
#
# creates "pi0:eff40_May2020Fit" ParticleList
stdPi0s('eff40_May2020Fit', path=my_path)

# reconstruct D0 -> pi0 pi0 decay
# keep only candidates with 1.7 < M(pi0pi0) < 2.0 GeV
reconstructDecay('D0:pi0pi0 -> pi0:eff40_May2020Fit pi0:eff40_May2020Fit', '1.7 < M < 2.0', path=my_path)

# perform mass fit using KFit
# Reject the candidates with failed fit.
kFit('D0:pi0pi0', 0.0, 'mass', path=my_path)

# reconstruct B0 -> D0 pi0 decay
# keep only candidates with Mbc > 5.24 GeV
# and -1 < Delta E < 1 GeV
reconstructDecay('B0:all -> D0:pi0pi0 pi0:eff40_May2020Fit', '5.24 < Mbc < 5.29 and abs(deltaE) < 1.0', path=my_path)

# perform MC matching (MC truth association)
matchMCTruth('B0:all', path=my_path)

# Select variables that we want to store to ntuple
B0_vars = vc.inv_mass + vc.mc_truth + \
    vu.create_aliases_for_selected(
        vc.inv_mass + vc.mc_truth,
        'B0 -> ^D0 ^pi0') + \
    vu.create_aliases_for_selected(
        vc.cluster, 'B0 -> D0 [pi0 -> ^gamma ^gamma]')

pi0_vars = vc.mc_truth + vc.kinematics + \
    ['extraInfo(BDT)', 'decayAngle(0)'] + \
    vu.create_aliases_for_selected(
        vc.cluster + vc.kinematics, 'pi0 -> ^gamma ^gamma')

# Saving variables to ntuple
output_file = 'B2A401-KFit-MassFit.root'
variablesToNtuple('B0:all', B0_vars,
                  filename=output_file, treename='b0', path=my_path)
variablesToNtuple('pi0:eff40_May2020Fit', pi0_vars,
                  filename=output_file, treename='pi0', path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
