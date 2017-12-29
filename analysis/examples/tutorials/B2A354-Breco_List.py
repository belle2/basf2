#!/usr/bin/env/python3
# -*- coding: utf-8 -*-

#####################################
#
#   Hadronic B Skim
# This skim reconstructed B+ and B0 decays via hadronic modes and returns a list #of Bmesons which is then written to a uDST file.
#
# R.Cheaib, 30/05/2017
#
######################################

from basf2 import *
from modularAnalysis import *


def Breco():
    # combine B+ and B0 lists and return list of reconstructed B mesons
    Lists = []
    Lists += BplusHadronic()
    Lists += B0Hadronic()

    return Lists


def D0ToYpZm():

    # define D0 mass cuts
    Dcuts = '1.8 < InvM <1.9'

# define D0 channels
    D0channels = ['K-:loose  pi+:loose',
                  'pi+:loose pi-:loose'
                  ]

# define D0 list  and reconstruct D0 decay modes
    D0List = []
    for chID, channel in enumerate(D0channels):
        reconstructDecay('D0:' + str(chID) + ' -> ' + channel, Dcuts, chID)
        D0List.append('D0:' + str(chID))
    copyLists('D0:loose', D0List)
    return D0List


def BplusHadronic():
    # define Mbc cut for B+ list
    Bcuts = '5.24<Mbc<5.29 '

# get D0 meson list
    D0List = D0ToYpZm()

# define B+ list and reconstruct B+ decay modes
    BplusList = []

    for chID, channel in enumerate(D0List):
        reconstructDecay('B+:' + str(chID) + ' -> pi+:all ' + channel, Bcuts, chID)
        BplusList.append('B+:' + str(chID))

    return BplusList


def B0Hadronic():
    # apply Mbc cuts
    Bcuts = '5.24 < Mbc < 5.29'
# define hadronic B decay modes
    B0channels = ['D0:loose pi+:loose pi-:loose',
                  'D0:loose K+:loose K-:loose'
                  ]
# reconstruct B0 hadronic decay modes and append to the B0 list
    B0List = []
    for chID, channel in enumerate(B0channels):
        reconstructDecay('B0:' + str(chID) + ' -> ' + channel, Bcuts, chID)
        B0List.append('B0:' + str(chID))

    return B0List
