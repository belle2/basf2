#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 2 Body skims
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 03 May 2018
#######################################################

from basf2 import *
from modularAnalysis import *


def CharmlessHad2BodyB0List():
    """
    List definitions for all 2 body neutral B modes.
    """
    applyEventCuts('R2EventLevel < 0.5 and nTracks >= 3')
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_PPChannels = ['pi+:loose pi-:loose',  # 1
                     'pi0:skim pi0:skim',  # 2
                     'K+:loose pi-:loose',  # 3
                     'K+:loose K-:loose',  # 4
                     'eta:loose pi0:skim',  # 5
                     'eta:loose eta:loose',  # 6
                     'eta\':loose pi0:skim',  # 7
                     'eta\':loose eta\':loose',  # 8
                     'eta\':loose eta:loose',  # 9
                     'K_S0:all pi0:skim']  # 10

    B0_PVChannels = ['rho0:loose pi0:skim',  # 1
                     'rho-:loose pi+:loose',  # 2
                     'K+:loose rho-:loose',  # 3
                     'eta\':loose rho0:loose',  # 4
                     'eta:loose rho0:loose',  # 5
                     'omega:loose eta:loose',  # 6
                     'omega:loose eta\':loose',  # 7
                     'phi:loose pi0:skim',  # 8
                     'phi:loose eta:loose',  # 9
                     'phi:loose eta\':loose',  # 10
                     'omega:loose pi0:skim',  # 11
                     'eta\':loose K*0:loose',  # 12
                     'eta:loose K*0:loose',  # 13
                     'K*+:loose pi-:loose',  # 14
                     'K*0:loose pi0:skim',  # 15
                     'K*+:loose K-:loose']  # 16

    B0_VVChannels = ['rho0:loose rho0:loose',  # 1
                     'rho+:loose rho-:loose',  # 2
                     'omega:loose rho0:loose',  # 3
                     'omega:loose omega:loose',  # 4
                     'phi:loose rho0:loose',  # 5
                     'phi:loose omega:loose',  # 6
                     'phi:loose phi:loose',  # 7
                     'omega:loose K*0:loose',  # 8
                     'K*0:loose rho0:loose',  # 9
                     'K*+:loose rho-:loose',  # 10
                     'K*0:loose phi:loose',  # 11
                     'K*0:loose anti-K*0:loose',  # 12
                     'K*0:loose K*0:loose',  # 13
                     'K*+:loose K*-:loose']  # 14

    B0_PSChannels = ['eta\':loose f_0:loose',  # 1
                     'eta:loose f_0:loose']  # 2

    B0_VSChannels = ['omega:loose f_0:loose',  # 1
                     'phi:loose f_0:loose',  # 2
                     'rho0:loose f_0:loose',  # 3
                     'K*0:loose f_0:loose']  # 4

    B0_SSChannels = ['f_0:loose f_0:loose']  # 1

    B0PPList = []
    for chID, channel in enumerate(B0_PPChannels):
        reconstructDecay('B0:HAD_b2PP' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PPList.append('B0:HAD_b2PP' + str(chID))

    B0PVList = []
    for chID, channel in enumerate(B0_PVChannels):
        reconstructDecay('B0:HAD_b2PV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PVList.append('B0:HAD_b2PV' + str(chID))

    B0VVList = []
    for chID, channel in enumerate(B0_VVChannels):
        reconstructDecay('B0:HAD_b2VV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0VVList.append('B0:HAD_b2VV' + str(chID))

    B0PSList = []
    for chID, channel in enumerate(B0_PSChannels):
        reconstructDecay('B0:HAD_b2PS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PSList.append('B0:HAD_b2PS' + str(chID))

    B0VSList = []
    for chID, channel in enumerate(B0_VSChannels):
        reconstructDecay('B0:HAD_b2VS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0VSList.append('B0:HAD_b2VS' + str(chID))

    B0SSList = []
    for chID, channel in enumerate(B0_SSChannels):
        reconstructDecay('B0:HAD_b2SS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0SSList.append('B0:HAD_b2SS' + str(chID))

    copyLists('B0:2BodyB0', B0PPList + B0PVList + B0VVList + B0PSList + B0VSList + B0SSList)

    List = ['B0:2BodyB0']
    return List


def CharmlessHad2BodyBmList():
    """
    List definitions for all 2 body charged B modes.
    """
    applyEventCuts('R2EventLevel < 0.5 and nTracks >= 3')
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    Bm_PPChannels = ['pi-:loose pi0:skim',  # 1
                     'K-:loose pi0:skim',  # 2
                     'eta:loose pi-:loose',  # 3
                     'eta\':loose pi-:loose',  # 4
                     'eta\':loose K-:loose',  # 5
                     'eta:loose K-:loose']  # 6

    Bm_PVChannels = ['rho0:loose pi-:loose',  # 1
                     'rho-:loose pi0:skim',  # 2
                     'K-:loose rho0:loose',  # 3
                     'omega:loose pi-:loose',  # 4
                     'eta:loose rho-:loose',  # 5
                     'eta\':loose rho-:loose',  # 6
                     'phi:loose pi-:loose',  # 7
                     'eta\':loose K*-:loose',  # 8
                     'eta:loose K*-:loose',  # 9
                     'omega:loose K-:loose',  # 10
                     'K*0:loose pi-:loose',  # 11
                     'K*-:loose pi0:skim',  # 12
                     'K-:loose phi:loose']  # 13

    Bm_VVChannels = ['rho-:loose rho0:loose',  # 1
                     'omega:loose rho-:loose',  # 2
                     'phi:loose rho-:loose',  # 3
                     'omega:loose K*-:loose',  # 4
                     'K*-:loose rho0:loose',  # 5
                     'K*0:loose rho-:loose',  # 6
                     'K*-:loose K*0:loose',  # 7
                     'K*-:loose phi:loose']  # 8

    Bm_PSChannels = ['pi-:loose f_0:loose',  # 1
                     'f_0:loose K-:loose']  # 2

    Bm_VSChannels = ['rho-:loose f_0:loose',  # 1
                     'K*-:loose f_0:loose']  # 2

    BmPPList = []
    for chID, channel in enumerate(Bm_PPChannels):
        reconstructDecay('B-:HAD_b2PP' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPPList.append('B-:HAD_b2PP' + str(chID))

    BmPVList = []
    for chID, channel in enumerate(Bm_PVChannels):
        reconstructDecay('B-:HAD_b2PV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPVList.append('B-:HAD_b2PV' + str(chID))

    BmVVList = []
    for chID, channel in enumerate(Bm_VVChannels):
        reconstructDecay('B-:HAD_b2VV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmVVList.append('B-:HAD_b2VV' + str(chID))

    BmPSList = []
    for chID, channel in enumerate(Bm_PSChannels):
        reconstructDecay('B-:HAD_b2PS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPSList.append('B-:HAD_b2PS' + str(chID))

    BmVSList = []
    for chID, channel in enumerate(Bm_VSChannels):
        reconstructDecay('B-:HAD_b2VS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmVSList.append('B-:HAD_b2VS' + str(chID))

    copyLists('B-:2BodyBm', BmPPList + BmPVList + BmVVList + BmPSList + BmVSList)

    List = ['B-:2BodyBm']
    return List
