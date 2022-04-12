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
# This tutorial shows how to apply event based selections                #
# before filling particle lists, to reject useless events                #
# without further processing. In this case the cut is on                 #
# the R2 variable, which for bb events is below 0.3,                     #
# while in continuum events can reach larger values.                     #
#                                                                        #
# The example A305 demonstrates how to reconstruct the                   #
# B meson decay with missing energy, e.g. :                              #
#                                                                        #
# Y(4S) -> Btag- Bsig+                                                   #
#                 |                                                      #
#                 +-> mu+ nu                                             #
#                                                                        #
# The original example has been modified to apply the                    #
# event based selection, and the ntuples are filled.                     #
# While signal/charged samples have R2 distributions below               #
# 0.3, ccbar sample has a larger distribution and the                    #
# initial selection reduces the processing time.                         #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import stdCharged as stdc
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdstList(filelist=[b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False)],
                 path=my_path)

# We want to apply cut on event shape. For this, we are creating events shape object
# First, create a list of good tracks (using the pion mass hypothesis)
# and good gammas with very minimal cuts
ma.fillParticleList(decayString='pi+:goodtracks',
                    cut='pt> 0.1',
                    path=my_path)
ma.fillParticleList(decayString='gamma:goodclusters',
                    cut='E > 0.1',
                    path=my_path)

# Second, create event shape
ma.buildEventShape(inputListNames=['pi+:goodtracks', 'gamma:goodclusters'],
                   allMoments=True,
                   foxWolfram=True,
                   harmonicMoments=True,
                   cleoCones=True,
                   thrust=True,
                   collisionAxis=True,
                   jets=True,
                   sphericity=True,
                   checkForDuplicates=False,
                   path=my_path)

# Apply a selection at the event level, to avoid
# processing useless events
ma.applyEventCuts(cut='foxWolframR2 < 0.3', path=my_path)


# The following lines cut&pasted from A304

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdPi(listtype='loose', path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdc.stdK(listtype='loose', path=my_path)
# creates "mu+:loose" ParticleList (and c.c.)
stdc.stdMu(listtype='loose', path=my_path)


# creates "pi0:eff40_May2020Fit" ParticleList
stdPi0s(listtype='eff40_May2020Fit', path=my_path)


# 1. reconstruct D0 in multiple decay modes
ma.reconstructDecay(decayString='D0:ch1 -> K-:loose pi+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=1,
                    path=my_path)

ma.reconstructDecay(decayString='D0:ch2 -> K-:loose pi+:loose pi0:eff40_May2020Fit',
                    cut='1.8 < M < 1.9',
                    dmID=2,
                    path=my_path)

ma.reconstructDecay(decayString='D0:ch3 -> K-:loose pi+:loose pi+:loose pi-:loose',
                    cut='1.8 < M < 1.9',
                    dmID=3,
                    path=my_path)

ma.reconstructDecay(decayString='D0:ch4 -> K-:loose K+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=4,
                    path=my_path)

ma.reconstructDecay(decayString='D0:ch5 -> pi-:loose pi+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=5,
                    path=my_path)

# 2. merge the D0 lists together into one single list
ma.copyLists(outputListName='D0:all',
             inputListNames=['D0:ch1', 'D0:ch2', 'D0:ch3', 'D0:ch4', 'D0:ch5'],
             path=my_path)


# 3. reconstruct B+ -> anti-D0 pi+ decay
ma.reconstructDecay(decayString='B+:tag -> anti-D0:all pi+:loose',
                    cut='5.24 < Mbc < 5.29 and abs(deltaE) < 1.0',
                    dmID=1,
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='B+:tag',
                path=my_path)


# 3. reconstruct Upsilon(4S) -> Btag+ Bsig- -> Btag+ mu-
ma.reconstructDecay(decayString='Upsilon(4S) -> B-:tag mu+:loose',
                    cut="",
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='Upsilon(4S)', path=my_path)

# 5. build rest of the event
ma.buildRestOfEvent(target_list_name='Upsilon(4S)', path=my_path)

d_vars = vc.mc_truth + vc.kinematics + vc.inv_mass + ['foxWolframR2']
mu_vars = vc.mc_truth

b_vars = vc.mc_truth + \
    vc.deltae_mbc + \
    vu.create_aliases_for_selected(list_of_variables=d_vars,
                                   decay_string='B- -> ^D0 pi-') + \
    vu.create_aliases(list_of_variables=['decayModeID'],
                      wrapper='daughter(0,extraInfo({variable}))',
                      prefix="D0") + \
    ['foxWolframR2']

u4s_vars = vc.mc_truth + \
    vc.roe_multiplicities + \
    vc.recoil_kinematics + \
    vc.extra_energy + \
    vc.kinematics + \
    vu.create_aliases_for_selected(list_of_variables=b_vars,
                                   decay_string='Upsilon(4S) -> ^B- mu+') + \
    vu.create_aliases_for_selected(list_of_variables=mu_vars,
                                   decay_string='Upsilon(4S) -> B- ^mu+')


# 7. Saving variables to ntuple
rootOutputFile = 'B2A307-BasicEventWiseNtupleSelection.root'
ma.variablesToNtuple(decayString='B-:tag',
                     variables=b_vars,
                     filename=rootOutputFile,
                     treename='btag',
                     path=my_path)
ma.variablesToNtuple(decayString='Upsilon(4S)',
                     variables=u4s_vars,
                     filename=rootOutputFile,
                     treename='btagbsig',
                     path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
