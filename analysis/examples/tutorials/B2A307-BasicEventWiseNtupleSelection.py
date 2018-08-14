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
# It is possible to run the example with the signal from
# A101, with the charged background or with the ccbar BG;
# while signal/charged samples have R2 distributins below
# 0.3, ccbar sample has a larger distribution and the
# initial selection reduces the processing time.
#
# Since the example needs files from MC8 as background,
# it can only run at KEK for such cases.
#
# Contributors: A. Zupanc (June 2014)
#               S. Spataro (October 2017)
#               I. Komarov (December 2017)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import copyLists
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import buildRestOfEvent
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from stdCharged import *
from stdPi0s import stdPi0s

# load data
# Signal file from A104 reconstructed with background
inputMdst('default', 'B2A101-Y4SEventGeneration-gsim-BKGx1.root')

# Background file from MC9 generic sample - charged
# inputMdst('default',
# 	'/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002167/e0000/4S/r00000/charged/sub00/mdst_000001_prod00002167_task00000001.root')

# Background file from MC9 generic sample - ccbar
# inputMdst('default',
# 	'/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002171_task00000001.root')

# Apply a selection at the event level, to avoid
# processing useless events
applyEventCuts('R2EventLevel < 0.3')

# The following lines cut&pasted from A304

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()
# creates "mu+:loose" ParticleList (and c.c.)
stdLooseMu()

# creates "pi0:looseFit" ParticleList
stdPi0s('looseFit')

# 1. reconstruct D0 in multiple decay modes
reconstructDecay('D0:ch1 -> K-:loose pi+:loose', '1.8 < M < 1.9', 1)
reconstructDecay('D0:ch2 -> K-:loose pi+:loose pi0:looseFit', '1.8 < M < 1.9', 2)
reconstructDecay('D0:ch3 -> K-:loose pi+:loose pi+:loose pi-:loose', '1.8 < M < 1.9', 3)
reconstructDecay('D0:ch4 -> K-:loose K+:loose', '1.8 < M < 1.9', 4)
reconstructDecay('D0:ch5 -> pi-:loose pi+:loose', '1.8 < M < 1.9', 5)

# merge the D0 lists together into one single list
copyLists('D0:all', ['D0:ch1', 'D0:ch2', 'D0:ch3', 'D0:ch4', 'D0:ch5'])

# 2. reconstruct Btag+ -> anti-D0 pi+
reconstructDecay('B+:tag -> anti-D0:all pi+:loose', '5.2 < Mbc < 5.29 and abs(deltaE) < 1.0', 1)
matchMCTruth('B+:tag')

# 3. reconstruct Upsilon(4S) -> Btag+ Bsig- -> Btag+ mu-
reconstructDecay('Upsilon(4S) -> B-:tag mu+:loose', "")

# perform MC matching (MC truth asociation)
matchMCTruth('Upsilon(4S)')

# 5. build rest of the event
buildRestOfEvent('Upsilon(4S)')

# 6. Dump info to ntuple
import variableCollections as vc

dvars = vc.mc_truth + vc.kinematics + vc.inv_mass + ['R2EventLevel']
muvars = vc.mc_truth

bvars = vc.mc_truth + vc.deltae_mbc + \
    vc.convert_to_all_selected_vars(dvars,
                                    'B- -> ^D0 pi-') + \
    vc.wrap_list(['decayModeID'], 'daughter(0,extraInfo(variable))', "D") + \
    ['R2EventLevel']

u4svars = vc.mc_truth + vc.roe_multiplicities + vc.recoil_kinematics + vc.extra_energy + vc.kinematics + \
    vc.convert_to_all_selected_vars(bvars, 'Upsilon(4S) -> ^B- mu+') + \
    vc.convert_to_all_selected_vars(dvars, 'Upsilon(4S) -> [B- -> ^D0 pi-] mu+') + \
    vc.convert_to_all_selected_vars(muvars, 'Upsilon(4S) -> B- ^mu+')


# 7. Saving variables to ntuple
from modularAnalysis import variablesToNtuple
rootOutputFile = 'B2A307-BasicEventWiseNtupleSelection.root'
variablesToNtuple('B-:tag', bvars,
                  filename=rootOutputFile, treename='btag')
variablesToNtuple('Upsilon(4S)', u4svars,
                  filename=rootOutputFile, treename='btagbsig')

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
