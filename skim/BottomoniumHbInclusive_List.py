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


# RETENTION RATE TOO HIGH: PLEASE RECONSIDER

def HbInclusiveList():
    Hbcuts = '0.4 < M < 0.6 and 9.8 < mRecoil < 10.'

    cutAndCopyList('gamma:soft', 'gamma:skim', 'E<2')
    applyCuts('gamma:soft', 'R2EventLevel < 0.5')

    fillParticleList('pi+:pi0', 'pt<0.5 and pt>0.05')
    reconstructDecay('pi0:skim -> gamma:soft gamma:soft', '0.125 < M < 0.14')

    Eta_Channels = ['pi+:pi0 pi-:pi0 pi0:skim',
                    'gamma:soft gamma:soft']

    HbList = []
    for chID, channel in enumerate(Eta_Channels):
        reconstructDecay('eta:all' + str(chID) + ' -> ' + channel, Hbcuts, chID)
        HbList.append('eta:all' + str(chID))

    Lists = HbList
    return Lists
