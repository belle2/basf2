#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to reconstruct the
# charm meson in various decay modes and print out
# ParticleList summary at the end:
# - fraction if events with at least one candidate
# - overlaps between different lists
# - ...
#
# The following charm meson decays are reconstructed:
# - D0  -> K- pi+
#       -> K- pi+ pi0
#       -> K- pi+ pi+ pi-
#
# - D+  -> K- pi+ pi+
#
# - Ds+ -> K- K+ pi+
#
# Note: This tutorial uses generic MC therefore it can be
# ran only on KEKCC.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import fillParticleList
from modularAnalysis import reconstructDecay
from modularAnalysis import copyLists
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import summaryOfLists
from stdCharged import *
from stdFSParticles import goodPi0

filelistMIX = ['/hsm/belle2/bdata/MC/generic/mixed/mcprod1405/BGx1/mc35_mixed_BGx1_s01/mixed_e0001r0010_s01_BGx1.mdst.root']
filelistCHG = ['/hsm/belle2/bdata/MC/generic/charged/mcprod1405/BGx1/mc35_charged_BGx1_s01/charged_e0001r0010_s01_BGx1.mdst.root']
filelistCC = ['/hsm/belle2/bdata/MC/generic/ccbar/mcprod1405/BGx1/mc35_ccbar_BGx1_s01/ccbar_e0001r0010_s01_BGx1.mdst.root']
filelistSS = ['/hsm/belle2/bdata/MC/generic/ssbar/mcprod1405/BGx1/mc35_ssbar_BGx1_s01/ssbar_e0001r0010_s01_BGx1.mdst.root']
filelistDD = ['/hsm/belle2/bdata/MC/generic/ddbar/mcprod1405/BGx1/mc35_ddbar_BGx1_s01/ddbar_e0001r0010_s01_BGx1.mdst.root']
filelistUU = ['/hsm/belle2/bdata/MC/generic/uubar/mcprod1405/BGx1/mc35_uubar_BGx1_s01/uubar_e0001r0010_s01_BGx1.mdst.root']

inputMdstList('MC5', filelistMIX + filelistCHG + filelistCC + filelistSS + filelistDD + filelistUU)

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()

# creates "pi0:all" and "pi0:good" ParticleLists
# pi0:all candidates are created form all good ECL clusters
# while pi0:good have to pass good cut
# on the BoostedDecisionTree output
goodPi0()

# 1. reconstruct D0 in multiple decay modes
reconstructDecay('D0:ch1 -> K-:loose pi+:loose', '1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5', 1)
reconstructDecay('D0:ch2 -> K-:loose pi+:loose pi0:good', '1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5', 2)
reconstructDecay('D0:ch3 -> K-:loose pi+:loose pi+:loose pi-:loose', '1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5', 3)

# merge the D0 lists together into one single list
copyLists('D0:all', ['D0:ch1', 'D0:ch2', 'D0:ch3'])

# 2. reconstruct D+ -> K- pi+ pi+
reconstructDecay('D+:kpipi -> K-:loose pi+:loose pi+:loose', '1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5', 1)

# 3. reconstruct Ds+ -> K- K+ pi+
reconstructDecay('D_s+:kkpi -> K-:loose K+:loose pi+:loose', '1.9 < M < 2.0 and 2.5 < useCMSFrame(p) < 5.5', 1)

# perform MC matching (MC truth asociation)
matchMCTruth('D0:all')
matchMCTruth('D+:kpipi')
matchMCTruth('D_s+:kkpi')

# print out summary of lists
# first for D0 lists only
summaryOfLists(['D0:ch1', 'D0:ch2', 'D0:ch3'])
# and for all charm
summaryOfLists(['D0:all', 'D+:kpipi', 'D_s+:kkpi'])

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
