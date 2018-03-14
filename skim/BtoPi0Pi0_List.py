#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B0 -> pi0pi0 skims
# Here is a change
# K. Smith
#
#######################################################

from basf2 import *
from modularAnalysis import *


def BtoPi0Pi0List():
    Bcuts = '5.24 < Mbc < 5.29 and abs(deltaE) < 0.5'

    B0_Channels = ['pi0:skim pi0:skim']

    B0List = []
    for chID, channel in enumerate(B0_Channels):
        reconstructDecay('B0:Pi0Pi0' + ' -> ' + channel, Bcuts, chID)
        B0List.append('B0:Pi0Pi0')

    Lists = B0List
    return Lists
