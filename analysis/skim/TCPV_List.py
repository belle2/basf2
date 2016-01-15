#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *


def TCPVList():
    btotcpvcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'
    reconstructDecay('J/psi:ee -> e+:all e-:all', '2.8 < M < 3.1')
    reconstructDecay('J/psi:mumu -> mu+:all mu-:all', '2.8 < M < 3.1')
    copyLists('J/psi:all', ['J/psi:ee', 'J/psi:mumu'])

    bd_Channels = ['J/psi:all K_S0:all',
                   'phi:loose K_S0:all',
                   'eta\':loose K_S0:all',
                   'eta:loose K_S0:all',
                   'K_S0:all K_S0:all K_S0:all']

    bdList = []
    for chID, channel in enumerate(bd_Channels):
        reconstructDecay('B0:TCPV' + str(chID) + ' -> ' + channel, btotcpvcuts, chID)
        bdList.append('B0:TCPV' + str(chID))

    tcpvLists = bdList
    return tcpvLists
