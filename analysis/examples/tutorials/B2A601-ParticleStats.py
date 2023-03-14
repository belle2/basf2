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
# charm meson in various decay modes and print out                       #
# ParticleList summary at the end:                                       #
# - fraction if events with at least one candidate                       #
# - overlaps between different lists                                     #
# - ...                                                                  #
#                                                                        #
# The following charm meson decays are reconstructed:                    #
# - D0  -> K- pi+                                                        #
#       -> K- pi+ pi0                                                    #
#       -> K- pi+ pi+ pi-                                                #
#                                                                        #
# - D+  -> K- pi+ pi+                                                    #
#                                                                        #
# - Ds+ -> K- K+ pi+                                                     #
#                                                                        #
# Note: Running over all samples might take time.                        #
# Consider manual limit of the number of processed events:               #
#                                                                        #
# > basf2 B2A601-ParticleStats.py -n 1000                                #
#                                                                        #
##########################################################################

import basf2 as b2
import modularAnalysis as ma
import stdCharged as stdc
from stdPi0s import stdPi0s

# create path
my_path = b2.create_path()

# load input ROOT file
ma.inputMdst(filename=b2.find_file('B2pi0D_D2hh_D2hhh_B2munu.root', 'examples', False),
             path=my_path)

# create and fill final state ParticleLists
# use standard lists
# creates "pi+:loose" ParticleList (and c.c.)
stdc.stdPi(listtype='loose', path=my_path)
# creates "K+:loose" ParticleList (and c.c.)
stdc.stdK(listtype='loose', path=my_path)


stdPi0s(listtype='eff40_May2020Fit', path=my_path)

# 1. reconstruct D0 in multiple decay modes
ma.reconstructDecay(decayString='D0:ch1 -> K-:loose pi+:loose',
                    cut='1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5',
                    dmID=1,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch2 -> K-:loose pi+:loose pi0:eff40_May2020Fit',
                    cut='1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5',
                    dmID=2,
                    path=my_path)
ma.reconstructDecay(decayString='D0:ch3 -> K-:loose pi+:loose pi+:loose pi-:loose',
                    cut='1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5',
                    dmID=3,
                    path=my_path)

# merge the D0 lists together into one single list
ma.copyLists(outputListName='D0:all', inputListNames=['D0:ch1', 'D0:ch2', 'D0:ch3'], path=my_path)

# 2. reconstruct D+ -> K- pi+ pi+
ma.reconstructDecay(decayString='D+:kpipi -> K-:loose pi+:loose pi+:loose',
                    cut='1.8 < M < 1.9 and 2.5 < useCMSFrame(p) < 5.5',
                    dmID=1,
                    path=my_path)

# 3. reconstruct Ds+ -> K- K+ pi+
ma.reconstructDecay(decayString='D_s+:kkpi -> K-:loose K+:loose pi+:loose',
                    cut='1.9 < M < 2.0 and 2.5 < useCMSFrame(p) < 5.5',
                    dmID=1,
                    path=my_path)

# perform MC matching (MC truth association)
ma.matchMCTruth(list_name='D0:all', path=my_path)
ma.matchMCTruth(list_name='D+:kpipi', path=my_path)
ma.matchMCTruth(list_name='D_s+:kkpi', path=my_path)

# print out summary of lists
# first for D0 lists only
ma.summaryOfLists(particleLists=['D0:ch1', 'D0:ch2', 'D0:ch3'], path=my_path)
# and for all charm
ma.summaryOfLists(particleLists=['D0:all', 'D+:kpipi', 'D_s+:kkpi'], path=my_path)

# Process the events
b2.process(my_path)

# print out the summary
print(b2.statistics)
