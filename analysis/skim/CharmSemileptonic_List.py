#!/usr/bin/evn python3
# -*- coding: utf-8 -*-

##################################################
#
# Charm SL skims
# J. Bennett, 5/Oct/2016
#
##################################################

from basf2 import *
from modularAnalysis import *


def CharmSemileptonicList():
    DstarSLcuts = 'massDifference(0)<0.16'
    antiD0SLcuts = '1.5 < M < 2.2'

    DstarSLRecoilChannels = ['D0:all pi+:all',
                             'D0:all pi+:all pi0:all',
                             'D0:all pi+:all pi+:all pi-:all',
                             'D+:all pi0:all',
                             'D+:all pi+:all pi-:all',
                             'D+:all pi+:all pi-:all pi0:all',
                             ]

    antiD0List = []
    for chID, channel in enumerate(DstarSLRecoilChannels):
        reconstructRecoil('D*-:SL' + str(chID) + ' -> ' + channel, DstarSLcuts, chID)
        reconstructRecoilDaughter('anti-D0:SL' + str(chID) + ' -> D*-:SL' + str(chID) +
                                  ' pi-:all', antiD0SLcuts, chID)
        antiD0List.append('anti-D0:SL' + str(chID))

    nueRecoilChannels = []
    for channel in antiD0List:
        nueRecoilChannels.append(channel + ' K+:all e-:all')
        nueRecoilChannels.append(channel + ' pi+:all e-:all')

    numuRecoilChannels = []
    for channel in antiD0List:
        numuRecoilChannels.append(channel + ' K+:all mu-:all')
        numuRecoilChannels.append(channel + ' pi+:all mu-:all')

    nueList = []
    for chID, channel in enumerate(nueRecoilChannels):
        reconstructRecoilDaughter('anti-nu_e:SL' + str(chID) + ' -> ' + channel, '', chID)
        nueList.append('anti-nu_e:SL' + str(chID))

    numuList = []
    for chID, channel in enumerate(numuRecoilChannels):
        reconstructRecoilDaughter('anti-nu_mu:SL' + str(chID) + ' -> ' + channel, '', chID)
        numuList.append('anti-nu_mu:SL' + str(chID))

    allLists = nueList + numuList
    return allLists
