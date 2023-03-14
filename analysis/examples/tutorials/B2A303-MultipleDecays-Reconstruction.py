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
# following decay chain involving several decay modes:                   #
#                                                                        #
# B- -> D0 pi-                                                           #
#       |                                                                #
#       +-> K- pi+                                                       #
#       +-> K- pi+ pi0                                                   #
#       +-> K- pi+ pi+                                                   #
#       +-> K- K+                                                        #
#       +-> pi- pi+                                                      #
#                                                                        #
# Note: This reconstruction is performed on generated level              #
# to speed up the reconstruction during the tutorial. However,           #
# the reconstruction can as well be performed using reconstructed        #
# final state particles.                                                 #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# create and fill final state ParticleLists
kaons = ('K-', '')
pions = ('pi-', '')
photons = ('gamma', '')
ma.fillParticleListsFromMC(decayStringsWithCuts=[kaons, pions, photons],
                           path=my_path)

# 1. reconstruct pi0 -> gamma gamma decay
# InvM is the sum of daughter momenta whereas M is the initialised PDG mass ...
ma.reconstructDecay(decayString='pi0 -> gamma gamma',
                    cut='0.1 < InvM < 0.15',
                    path=my_path)

# 2. reconstruct D0 in multiple decay modes
ma.reconstructDecay(decayString='D0:ch1 -> K- pi+',
                    cut='1.8 < M < 1.9',
                    dmID=1,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch2 -> K- pi+ pi0',
                    cut='1.8 < M < 1.9',
                    dmID=2,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch3 -> K- pi+ pi+ pi-',
                    cut='1.8 < M < 1.9',
                    dmID=3,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch4 -> K- K+',
                    cut='1.8 < M < 1.9',
                    dmID=4,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch5 -> pi+ pi-',
                    cut='1.8 < M < 1.9',
                    dmID=5,
                    path=my_path)

# merge the D0 lists together into one single list
ma.copyLists(outputListName='D0:all',
             inputListNames=['D0:ch1', 'D0:ch2', 'D0:ch3', 'D0:ch4', 'D0:ch5'],
             path=my_path)

# 3. reconstruct B+ -> anti-D0 pi+ decay
ma.reconstructDecay(decayString='B+:D0pi -> anti-D0:all pi+',
                    cut='5.24 < Mbc < 5.29 and abs(deltaE) < 1.0',
                    dmID=1,
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='B+:D0pi',
                path=my_path)

# Select variables that we want to store to ntuple
d_vars = vc.inv_mass + vc.kinematics
pi_vars = vc.kinematics
b_vars = vc.deltae_mbc + \
    vc.mc_truth + \
    vu.create_aliases_for_selected(list_of_variables=d_vars,
                                   decay_string='B+ -> ^anti-D0 pi+',
                                   prefix='D0') + \
    vu.create_aliases_for_selected(list_of_variables=pi_vars,
                                   decay_string='B+ -> anti-D0 ^pi+',
                                   prefix='pi') + \
    vu.create_aliases(list_of_variables=['decayModeID'],
                      wrapper='daughter(0,extraInfo({variable}))',
                      prefix="")

# Saving variables to ntuple
output_file = 'B2A303-MultipleDecays-Reconstruction.root'
ma.variablesToNtuple(decayString='B+:D0pi',
                     variables=b_vars,
                     filename=output_file,
                     treename='bp',
                     path=my_path)


# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
