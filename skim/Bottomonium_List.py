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


def HbInclusiveList():
    Hbcuts = '0.4 < M < 0.6 and 9.8 < mRecoil < 10.'

    cutAndCopyList('gamma:soft', 'gamma:loose', 'E<2')
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


def EtabInclusiveList():
    Etabcuts = 'mRecoil > 9 and mRecoil < 10'
    cutAndCopyList('gamma:soft', 'gamma:loose', 'E<2')
    cutAndCopyList('gamma:hb', 'gamma:loose', 'useCMSFrame(p)>0.4 and useCMSFrame(p)<0.5')
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
