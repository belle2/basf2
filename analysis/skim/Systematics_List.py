#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Sysetmatics Skims
# P. Urquijo, 1/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *


def SystematicsList():

    Lists = []
    Lists += JpsieeTagProbe()
    Lists += JpsimumuTagProbe()

    return Lists


def JpsieeTagProbe():
    Cuts = '2.7 < M < 3.4'
    Channel = 'e+:all e-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:eetagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:eetagprobe' + str(chID))
    return jpsiList


def JpsimumuTagProbe():
    Cuts = '2.8 < M < 3.4'
    Channel = 'mu+:all mu-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:mumutagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:mumutagprobe' + str(chID))
    return jpsiList
