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

# If retention rate could be further reduced, can combine this with Jpsiee


def SystematicsList():

    Lists = []
    Lists += JpsimumuTagProbe()

    return Lists


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
