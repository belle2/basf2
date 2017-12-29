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


# RETENTION RATE HIGH: IMPROVE CUTS

def EtabInclusiveList():
    Etabcuts = 'mRecoil > 9 and mRecoil < 10'
    cutAndCopyList('gamma:soft', 'gamma:skim', 'E<2')
    cutAndCopyList('gamma:hb', 'gamma:skim', 'useCMSFrame(p)>0.4 and useCMSFrame(p)<0.5')
    applyCuts('gamma:hb', 'R2EventLevel < 0.5')

    fillParticleList('pi+:pi0', 'pt<0.5 and pt>0.05')
    reconstructDecay('pi0:skim -> gamma:soft gamma:soft', '0.125 < M < 0.14')
    reconstructDecay('eta:3pi -> pi+:pi0 pi-:pi0 pi0:skim', '0.53 < M < 0.56 and 9.85 < mRecoil < 9.9')
    reconstructDecay('eta:gg -> gamma:soft gamma:soft', '0.53 < M < 0.56 and 9.85 < mRecoil < 9.92')

    EtabInclusive_Channels = ['eta:3pi gamma:hb',
                              'eta:gg gamma:hb']

    EtabInclusiveList = []
    for chID, channel in enumerate(EtabInclusive_Channels):
        reconstructDecay('junction:all' + str(chID) + ' -> ' + channel, Etabcuts, chID)
        EtabInclusiveList.append('junction:all' + str(chID))

    Lists = EtabInclusiveList
    return Lists
