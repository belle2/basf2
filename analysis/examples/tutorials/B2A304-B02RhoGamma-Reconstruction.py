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
# This tutorial demonstrates how to reconstruct the                      #
# following decay chain:                                                 #
#                                                                        #
# B0 -> rho gamma                                                        #
#       |                                                                #
#       +-> pi+ pi-                                                      #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
from stdPhotons import stdPhotons
import stdCharged as stdc

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(environmentType='default',
             filename=b2.find_file('B2rhogamma_rho2pipi.root', 'examples', False),
             path=my_path)

#
# creates "gamma:tight" ParticleList
stdPhotons(listtype='tight', path=my_path)

# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdPi(listtype='loose', path=my_path)

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
ma.reconstructDecay(decayString='rho0 -> pi+:loose pi-:loose',
                    cut='0.6 < M < 1.0',
                    path=my_path)

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
ma.reconstructDecay(decayString='B0 -> rho0 gamma:tight',
                    cut='5.2 < Mbc < 5.29 and abs(deltaE) < 2.0',
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='B0', path=my_path)

# Select variables that we want to store to ntuple

gamma_vars = vc.cluster + vc.mc_truth + vc.kinematics
rho_vars = vc.cluster + vc.mc_truth + vc.kinematics + vc.inv_mass
pi_vars = vc.pid + vc.track
b_vars = vc.kinematics + \
    vc.deltae_mbc + \
    vc.mc_truth + \
    vu.create_aliases_for_selected(list_of_variables=gamma_vars,
                                   decay_string='B0 -> rho0 ^gamma') + \
    vu.create_aliases_for_selected(list_of_variables=rho_vars,
                                   decay_string='B0 -> ^rho0 gamma') + \
    vu.create_aliases_for_selected(list_of_variables=pi_vars,
                                   decay_string='B0 -> [rho0 -> ^pi+ ^pi-] gamma')

# Saving variables to ntuple
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction.root'
ma.variablesToNtuple(decayString='B0',
                     variables=b_vars,
                     filename=rootOutputFile,
                     treename='b0',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
