#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmless Hadronic 3 Body hbds
#
# K. Smith (khsmith@student.unimelb.edu.au)
# Last updated 25 Jun 2018
#######################################################

from basf2 import *
from modularAnalysis import *


def CharmlessHad3BodyB0List():
    """
    List definitions for all 3 body neutral B modes.
    """

    applyEventCuts('R2EventLevel < 0.5 and nTracks >= 2')

    cutAndCopyList('pi0:hbd', 'pi0:skim', '0.12 < M < 0.14 and E > 0.25')
    cutAndCopyList('K_S0:hbd', 'K_S0:all', '0.48 < M < 0.51')

    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_PPPChannels = ['pi+:loose pi-:loose pi0:hbd',  # 1
                      'K+:loose pi-:loose pi0:hbd',  # 2
                      'K+:loose K-:loose pi0:hbd',  # 3
                      'K_S0:hbd K_S0:hbd pi0:hbd',  # 4
                      'K_S0:hbd K_S0:hbd eta:loose',  # 5
                      'K_S0:hbd K_S0:hbd K_S0:hbd']  # 6

    B0_PPVChannels = ['rho0:loose pi+:loose pi-:loose',  # 1
                      'rho0:loose K+:loose pi-:loose',  # 2
                      'phi:loose pi+:loose pi-:loose',  # 3
                      'K*0:loose pi+:loose pi-:loose',  # 4
                      'K*0:loose K+:loose K-:loose',  # 5
                      'K*0:loose K-:loose pi+:loose',  # 6
                      'K*0:loose K+:loose pi-:loose']  # 7

    B0_PPSChannels = ['f_0:loose pi+:loose pi-:loose',  # 1
                      'f_0:loose K+:loose pi-:loose']  # 2

    B0PPPList = []
    for chID, channel in enumerate(B0_PPPChannels):
        reconstructDecay('B0:HAD_b2PPP' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PPPList.append('B0:HAD_b2PPP' + str(chID))

    B0PPVList = []
    for chID, channel in enumerate(B0_PPVChannels):
        reconstructDecay('B0:HAD_b2PPV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PPVList.append('B0:HAD_b2PPV' + str(chID))

    B0PPSList = []
    for chID, channel in enumerate(B0_PPSChannels):
        reconstructDecay('B0:HAD_b2PPS' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0PPSList.append('B0:HAD_b2PPS' + str(chID))

    copyLists('B0:3BodyB0', B0PPPList + B0PPVList + B0PPSList)

    buildRestOfEvent('B0:3BodyB0')
    cleanMask = ('cleanMask', 'useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
    appendROEMasks('B0:3BodyB0', [cleanMask])
    buildContinuumSuppression('B0:3BodyB0', 'cleanMask')
    cutAndCopyList('B0:3BodySkim', 'B0:3BodyB0', 'abs(cosTBTO) < 0.9 and abs(cosTBz) < 0.85')

    List = ['B0:3BodySkim']
    return List


def CharmlessHad3BodyBmList():
    """
    List definitions for all 3 body charged B modes.
    """

    applyEventCuts('R2EventLevel < 0.5 and nTracks >= 2')

    cutAndCopyList('pi0:hbd', 'pi0:skim', '0.12 < M < 0.15 and E > 0.25')
    cutAndCopyList('K_S0:hbd', 'K_S0:all', '0.48 < M < 0.51')

    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    Bm_PPPChannels = ['K-:loose pi+:loose pi-:loose',  # 1
                      'K-:loose pi0:hbd pi0:hbd',  # 2
                      'K+:loose pi-:loose pi-:loose',  # 3
                      'K-:loose K+:loose pi-:loose',  # 4
                      'K-:loose K+:loose K-:loose',  # 5
                      'pi-:loose pi-:loose pi+:loose',  # 6
                      'pi-:loose pi0:hbd pi0:hbd',  # 7
                      'K-:loose K_S0:hbd K_S0:hbd',  # 8
                      'pi-:loose K_S0:hbd K_S0:hbd',  # 9
                      'K_S0:hbd pi-:loose pi0:hbd']  # 10

    Bm_PPVChannels = ['K*-:loose pi-:loose pi+:loose',  # 1
                      'K*-:loose pi-:loose K+:loose',  # 2
                      'K*-:loose K-:loose K+:loose',  # 3
                      'phi:loose K_S0:hbd pi-:loose']  # 4

    Bm_PVVChannels = ['K-:loose phi:loose phi:loose',  # 1
                      'omega:loose phi:loose K-:loose']  # 2

    BmPPPList = []
    for chID, channel in enumerate(Bm_PPPChannels):
        reconstructDecay('B-:HAD_b2PPP' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPPPList.append('B-:HAD_b2PPP' + str(chID))

    BmPPVList = []
    for chID, channel in enumerate(Bm_PPVChannels):
        reconstructDecay('B-:HAD_b2PPV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPPVList.append('B-:HAD_b2PPV' + str(chID))

    BmPVVList = []
    for chID, channel in enumerate(Bm_PVVChannels):
        reconstructDecay('B-:HAD_b2PVV' + str(chID) + ' -> ' + channel, Bcuts, chID)
        BmPVVList.append('B-:HAD_b2PVV' + str(chID))

    copyLists('B-:3BodyBm', BmPPPList + BmPPVList + BmPVVList)

    buildRestOfEvent('B-:3BodyBm')
    cleanMask = ('cleanMask', 'useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
    appendROEMasks('B-:3BodyBm', [cleanMask])
    buildContinuumSuppression('B-:3BodyBm', 'cleanMask')
    cutAndCopyList('B-:3BodySkim', 'B-:3BodyBm', 'abs(cosTBTO) < 0.9 and abs(cosTBz) < 0.85')

    List = ['B-:3BodySkim']
    return List
