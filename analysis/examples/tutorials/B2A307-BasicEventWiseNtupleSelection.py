#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#########################################################
#
# Stuck? Ask for help at questions.belle2.org
#
# This tutorial shows how to apply event based selections
# before filling particle lists, to reject useless events
# without further processing. In this case the cut is on
# the R2 variable, which for bb events is below 0.3,
# while in continuum events can reach larger values.
#
# The example A305 demonstrates how to reconstruct the
# B meson decay with missing energy, e.g. :
#
# Y(4S) -> Btag- Bsig+
#                 |
#                 +-> mu+ nu
#
# The original example has been modified to apply the
# event based selection, and the ntuples are filled
# also with the R2EventLevel variable.
# While signal/charged samples have R2 distributins below
# 0.3, ccbar sample has a larger distribution and the
# initial selection reduces the processing time.
#
# Contributors: A. Zupanc (June 2014)
#               S. Spataro (October 2017)
#               I. Komarov (December 2017)
#               I. Komarov (September 2018)
#
################################################################################

import basf2 as b2
import modularAnalysis as ma
import variables.collections as vc
import variables.utils as vu
import stdCharged as stdc
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdstList(environmentType='default',
                 filelist=[b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
                           b2.find_file('ccbar_background.root', 'examples', False)],
                 path=my_path)

# Apply a selection at the event level, to avoid
# processing useless events
ma.applyEventCuts(cut='R2EventLevel < 0.3', path=my_path)

# The following lines cut&pasted from A304

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdPi(listtype='loose', path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdc.stdK(listtype='loose', path=my_path)
# creates "mu+:loose" ParticleList (and c.c.)
stdc.stdMu(listtype='loose', path=my_path)


# creates "pi0:looseFit" ParticleList
stdPi0s(listtype='looseFit', path=my_path)

# 1. reconstruct D0 in multiple decay modes
ma.reconstructDecay(decayString='D0:ch1 -> K-:loose pi+:loose',
                    cut='1.8 < M < 1.9',
                    dmID=1,
                    path=my_path)

ma.reconstructDecay(decayString='D0:ch2 -> K-:loose pi+:loose pi0:looseFit',
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

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='B+:tag',
                path=my_path)


# 3. reconstruct Upsilon(4S) -> Btag+ Bsig- -> Btag+ mu-
ma.reconstructDecay(decayString='Upsilon(4S) -> B-:tag mu+:loose',
                    cut="",
                    path=my_path)

# perform MC matching (MC truth asociation)
ma.matchMCTruth(list_name='Upsilon(4S)', path=my_path)

# 5. build rest of the event
ma.buildRestOfEvent(target_list_name='Upsilon(4S)', path=my_path)

d_vars = vc.mc_truth + vc.kinematics + vc.inv_mass + ['R2EventLevel']
mu_vars = vc.mc_truth

b_vars = vc.mc_truth + \
    vc.deltae_mbc + \
    vu.create_aliases_for_selected(list_of_variables=d_vars,
                                   decay_string='B- -> ^D0 pi-') + \
    vu.create_aliases(list_of_variables=['decayModeID'],
                      wrapper='daughter(0,extraInfo(variable))',
                      prefix="D") + \
    ['R2EventLevel']

u4s_vars = vc.mc_truth + \
    vc.roe_multiplicities + \
    vc.recoil_kinematics + \
    vc.extra_energy + \
    vc.kinematics + \
    vu.create_aliases_for_selected(list_of_variables=b_vars,
                                   decay_string='Upsilon(4S) -> ^B- mu+') + \
    vu.create_aliases_for_selected(list_of_variables=d_vars,
                                   decay_string='Upsilon(4S) -> [B- -> ^D0 pi-] mu+') + \
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
