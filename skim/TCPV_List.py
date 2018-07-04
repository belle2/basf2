#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# TCPV skims
# P. Urquijo, 6/Jan/2015
# P. Urquijo 1/Oct/2016
######################################################


"""
Time dependent CP violation skim standalone for the analysis of TDCPV analysis in WG3 .
Physics channels : bd → qqs and bd → ccs .
Skim code : 13160100
for analysis users we will reconstruct again in the analysis steering file
The skim uses standard paticles from analysis package , exept one list of gamma:E15,
 of specific energy range, for 'B0 -> Kshort pi+ pi- gamma' channel.

Particle lists used :

phi:loose
k_S0:all
eta:loose
pi0:loose
ipi0:skim
rho0:loose
pi+:all
gamma:E15
omega:loose
J/psi: eeLoose
J/psi: mumuLoose
psi(2S): eeLoose
psi(2S): mumuloose
K*0:loose
phi:loose

"""
__author__ = " Reem Rasheed"

from basf2 import *
from modularAnalysis import *


def TCPVList():
    btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'

    bd_qqs_Channels = [
        'phi:loose K_S0:all',
        'eta\':loose K_S0:all',
        'eta:loose K_S0:all',
        'K_S0:all K_S0:all K_S0:all',
        'pi0:skim K_S0:all',
        'rho0:loose K_S0:all',
        'omega:loose K_S0:all',
        'f_0:loose K_S0:all',
        'pi0:skim pi0:skim K_S0:all',
        'phi:loose K_S0:all pi0:skim',
        'pi+:all pi-:all K_S0:all',
        'pi+:all pi-:all K_S0:all gamma:E15']

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
