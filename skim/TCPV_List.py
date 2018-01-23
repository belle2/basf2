#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 6/Jan/2015
# P. Urquijo 1/Oct/2016
######################################################

from basf2 import *
from modularAnalysis import *


def TCPVList():
    btotcpvcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    bd_qqs_Channels = [
        'phi:loose K_S0:all',
        'eta:loose K_S0:all',
        'eta:loose K_S0:all',
        'K_S0:all K_S0:all K_S0:all',
        'pi0:skim K_S0:all',
        'rho0:loose K_S0:all',
        'omega:loose K_S0:all',
        'f_0:loose K_S0:all',
        'pi0:skim pi0:skim K_S0:all',
        'phi:loose K_S0:all pi0:skim',
        'pi+:all pi-:all K_S0:all']

    bd_ccs_Channels = ['J/psi:eeLoose K_S0:all',
                       'J/psi:mumuLoose K_S0:all',
                       'psi(2S):eeLoose K_S0:all',
                       'psi(2S):mumuLoose K_S0:all',
                       'J/psi:eeLoose K*0:loose',
                       'J/psi:mumuLoose K*0:loose']

    bd_qqs_List = []
    for chID, channel in enumerate(bd_qqs_Channels):
        reconstructDecay('B0:TCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID)
        applyCuts('B0:TCPV_qqs' + str(chID), 'nTracks>4')
        bd_qqs_List.append('B0:TCPV_qqs' + str(chID))

    bd_ccs_List = []
    for chID, channel in enumerate(bd_ccs_Channels):
        reconstructDecay('B0:TCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID)
        applyCuts('B0:TCPV_ccs' + str(chID), 'nTracks>4')
        bd_ccs_List.append('B0:TCPV_ccs' + str(chID))

    tcpvLists = bd_qqs_List + bd_ccs_List
    return tcpvLists
