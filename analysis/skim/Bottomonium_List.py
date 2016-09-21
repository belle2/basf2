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

loadStdGoodPhoton()


def UpsilonList():
    Ycuts = ''
    fillParticleList('mu+:stiff', 'p<15 and p>3.5')
    fillParticleList('e+:stiff', 'p<15 and p>3.5')
    fillParticleList('pi+:soft', 'p<1.5 and pt>0.05')
    cutAndCopyList('gamma:soft', 'gamma:good', 'E>0.15')
    reconstructDecay('Upsilon:ee -> e+:stiff e-:stiff', 'M > 8')
    reconstructDecay('Upsilon:mumu -> mu+:stiff mu-:stiff', 'M > 8')
    copyLists('Upsilon:all', ['Upsilon:ee', 'Upsilon:mumu'])

    buildRestOfEvent('Upsilon:all')
    buildContinuumSuppression('Upsilon:all')
    applyCuts('Upsilon:all', 'R2 < 0.995')
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
#    fillParticleList('pi+:soft', 'p<1.5 and pt>0.05')
    cutAndCopyList('gamma:hard', 'gamma:good', 'E>3.5')

    Etab_Channels = ['gamma:hard gamma:hard']

    EtabList = []
    for chID, channel in enumerate(Etab_Channels):
        reconstructDecay('eta_b:all' + str(chID) + ' -> ' + channel, Etabcuts, chID)
        EtabList.append('eta_b:all' + str(chID))

    Lists = EtabList
    return Lists
