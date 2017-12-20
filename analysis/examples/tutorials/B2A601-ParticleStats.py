#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Stuck? Ask for help at questions.belle2.org
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
# Note 2: Running over all samples might take time.
# Consider manual limit of the number of processed events:
#
# > basf2 B2A601-ParticleStats.py -n 1000
#
# Contributors: A. Zupanc (June 2014),
#               I. Komarov (December 2017)
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
from stdFSParticles import stdPi0s

filelistMIX = ['/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002166\
/e0000/4S/r00000/mixed/sub00/mdst_000001_prod00002166_task00000001.root']
filelistCHG = ['/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002167\
/e0000/4S/r00000/charged/sub00/mdst_000001_prod00002167_task00000001.root']
filelistCC = ['/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171\
/e0000/4S/r00000/ccbar/sub00/mdst_000001_prod00002171_task00000001.root']
filelistSS = ['/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002170\
/e0000/4S/r00000/ssbar/sub00/mdst_000001_prod00002170_task00000001.root']
filelistDD = ['/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002169\
/e0000/4S/r00000/ddbar/sub00/mdst_000001_prod00002169_task00000001.root']
filelistUU = ['/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002168\
/e0000/4S/r00000/uubar/sub00/mdst_000001_prod00002168_task00000001.root']

inputMdstList('default', filelistMIX + filelistCHG + filelistCC + filelistSS + filelistDD + filelistUU)

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdLoosePi()
# creates "K+:loose" ParticleList (and c.c.)
stdLooseK()

stdPi0s('looseFit')

# 1. reconstruct D0 in multiple decay modes
reconstructDecay('D0:ch1 -> K-:loose pi+:loose', '1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5', 1)
reconstructDecay('D0:ch2 -> K-:loose pi+:loose pi0:looseFit', '1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5', 2)
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
