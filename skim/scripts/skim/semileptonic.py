#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

__authors__ = [
    "Racha Cheaib",
    "Hannah Wakeling"
]

from basf2 import *
from modularAnalysis import *


def SemileptonicList():

    __authors__ = [
        "Phillip Urquijo",
        "Racha Cheaib"
    ]

    """(Semi-)Leptonic Working Group skim for semi-leptonic analysis.
    To be used initially for for B semileptonic decays (B to D l v) (l= electron, muon)
    Skim code: 11160200

    Modes Reconstructed:
    *B+ -> D0 e+
    *B+ -> D0 mu+
    *B+ -> D*0 e+
    *B+ -> D*0 mu+
    *B0 ->  D+ e-
    *B0 ->  D+ mu-
    *B0 ->  D*+ e-
    *B0 ->  D*+ mu-

    Note: Uses D:all lists

    Cuts applied:
    *lepton momentum > 0.35 GeV
    *5.24 < B_Mbc < 5.29
    *| deltaE | < 0.5
    *nTracks > 4

    """
    cutAndCopyList('e-:SLB', 'e-:all', 'p>0.35', True)
    cutAndCopyList('mu-:SLB', 'mu-:all', 'p>0.35', True)
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    BplusChannels = ['D0:all e+:SLB',
                     'D0:all mu+:SLB',
                     'D*0:all e+:SLB',
                     'D*0:all mu+:SLB'
                     ]

    B0Channels = ['D+:all e-:SLB',
                  'D+:all mu-:SLB',
                  'D*+:all e-:SLB',
                  'D*+:all mu-:SLB'
                  ]

    bplusList = []
    for chID, channel in enumerate(BplusChannels):
        reconstructDecay('B+:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        applyCuts('B+:SL' + str(chID), 'nTracks>4')
        bplusList.append('B+:SL' + str(chID))

    b0List = []
    for chID, channel in enumerate(B0Channels):
        reconstructDecay('B0:SL' + str(chID) + ' -> ' + channel, Bcuts, chID)
        applyCuts('B+:SL' + str(chID), 'nTracks>4')
        b0List.append('B0:SL' + str(chID))

    allLists = b0List + bplusList
    return allLists


def PRList():
    """ (Semi-)Leptonic Working Group skim for partial reconstruction analysis.
    To be used initially for measuring B to D* l v using partial reconstruction.
    Skim code: 11110100

    Modes reconstructed:
    *B0:L1 ->  pi-:PR1 e+:PR1
    *B0:L2 ->  pi-:PR1 mu+:PR1

    Cuts applied:
    *electronID>0.5
    *muonID>0.5
    *lepton Momentum>1.5
    *R2EventLevel<0.5
    *nTracks>4
    """

    __authors__ = [
        "Lucien Cremaldi",
        "Racha Cheaib",
        "Romulus Godang"
    ]

    cutAndCopyList('e+:PR1', 'e+:all', 'useCMSFrame(p) > 1.50 and electronID > 0.5')
    cutAndCopyList('mu+:PR1', 'mu+:all', 'useCMSFrame(p) > 1.50 and muonID > 0.5')
    cutAndCopyList('pi-:PR1', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220\
')

    cutAndCopyList('e+:PR2', 'e+:all', '0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5')
    cutAndCopyList('mu+:PR2', 'mu+:all', '0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5')
    cutAndCopyList('pi-:PR2', 'pi-:all', 'pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160\
')

    reconstructDecay('B0:L1 ->  pi-:PR1 e+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 1)
    applyCuts('B0:L1', 'R2EventLevel<0.5 and nTracks>4')

    reconstructDecay('B0:L2 ->  pi-:PR1 mu+:PR1', 'useCMSFrame(daughterAngle(0,1))<0.00', 2)
    applyCuts('B0:L2', 'R2EventLevel<0.5 and nTracks>4')

    reconstructDecay('B0:L3 ->  pi-:PR2 e+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 3)
    applyCuts('B0:L3', 'R2EventLevel<0.5 and nTracks>4')

    reconstructDecay('B0:L4 ->  pi-:PR2 mu+:PR2', 'useCMSFrame(daughterAngle(0,1))<1.00', 4)
    applyCuts('B0:L4', 'R2EventLevel<0.5 and nTracks>4')

    PRList = ['B0:L1', 'B0:L2']

    return PRList
