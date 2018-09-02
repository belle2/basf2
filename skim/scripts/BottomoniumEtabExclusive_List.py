#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2015
#
######################################################
from basf2 import *
from modularAnalysis import *
from stdPhotons import *
stdPhotons('loose')


def EtabList():
    Etabcuts = 'M > 7 and M < 10'
    fillParticleList('gamma:hard', 'E>3.5')
#    cutAndCopyList('gamma:hard', 'gamma:loose', 'E>3.5') # for some unknown reason this line is crashing the code
    applyCuts('gamma:hard', 'R2EventLevel < 0.995')

    Etab_Channels = ['gamma:hard gamma:hard']

    EtabList = []
    for chID, channel in enumerate(Etab_Channels):
        reconstructDecay('eta_b:all' + str(chID) + ' -> ' + channel, Etabcuts, chID)
        EtabList.append('eta_b:all' + str(chID))

    Lists = EtabList
    return Lists
