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

# COULD YOU PLEASE USE STANDARD PARTICLE LISTS HERE?


def UpsilonList():
    Ycuts = ''
    fillParticleList('mu+:stiff', 'p<15 and p>3.5')
    fillParticleList('e+:stiff', 'p<15 and p>3.5')
    fillParticleList('pi+:soft', 'p<1.5 and pt>0.05')
    cutAndCopyList('gamma:soft', 'gamma:loose', 'E>0.15')
    reconstructDecay('Upsilon:ee -> e+:stiff e-:stiff', 'M > 8')
    reconstructDecay('Upsilon:mumu -> mu+:stiff mu-:stiff', 'M > 8')
    copyLists('Upsilon:all', ['Upsilon:ee', 'Upsilon:mumu'])

    applyCuts('Upsilon:all', 'R2EventLevel < 0.995')
    Upsilon_Channels = ['Upsilon:all pi+:soft',
                        'Upsilon:all gamma:soft']

    UpsilonList = []
    for chID, channel in enumerate(Upsilon_Channels):
        reconstructDecay('junction:all' + str(chID) + ' -> ' + channel, Ycuts, chID)

        UpsilonList.append('junction:all' + str(chID))

    Lists = UpsilonList
    return Lists
