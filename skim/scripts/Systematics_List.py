#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Sysetmatics Skims
# P. Urquijo, 1/Oct/2016
# Modified by Y. Kato, Mar/2018
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsList():

    Lists = []
    Lists += JpsimumuTagProbe()
    Lists += JpsieeTagProbe()
    Lists += PiKFromDstarList()
    return Lists


def PiKFromDstarList():
    D0Cuts = '1.81 < M < 1.91'
#   DstarCuts = 'massDifference(0)<0.16'
    DstarCuts = 'massDifference(0)<0.16 and useCMSFrame(p) > 1.5'

    D0Channel = ['K-:all pi+:all'
                 ]

    D0List = []
    for chID, channel in enumerate(D0Channel):
        reconstructDecay('D0:syst' + str(chID) + ' -> ' + channel, D0Cuts, chID)
        vertexRave('D0:syst' + str(chID), 0.0)
        D0List.append('D0:syst' + str(chID))

    DstarChannel = []
    for channel in D0List:
        DstarChannel.append(channel + ' pi+:all')

    DstarList = []
    for chID, channel in enumerate(DstarChannel):
        reconstructDecay('D*-:syst' + str(chID) + ' -> ' + channel, DstarCuts, chID)
        DstarList.append('D*-:syst' + str(chID))
        matchMCTruth('D*+:syst0')

    return DstarList


def JpsimumuTagProbe():
    #   Cuts = '2.8 < M < 3.4'
    Cuts = '2.7 < M < 3.4 and useCMSFrame(p) < 2.0'
    Channel = 'mu+:all mu-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:mumutagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:mumutagprobe' + str(chID))
    matchMCTruth('J/psi:mumutagprobe0')
    return jpsiList


def JpsieeTagProbe():
    #   Cuts = '2.7 < M < 3.4'
    Cuts = '2.7 < M < 3.4 and useCMSFrame(p) < 2.0'
    Channel = 'e+:all e-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:eetagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:eetagprobe' + str(chID))
    matchMCTruth('J/psi:eetagprobe0')
    return jpsiList
