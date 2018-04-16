#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Sysetmatics Skims
# P. Urquijo, 1/Oct/2016
# Modified by Y. Kato Mar/2018
######################################################

from basf2 import *
from modularAnalysis import *
# If the rention rate could be further reduced, we can combine this with Jpsi to mu mu


def SystematicsList():

    Lists = []
    Lists += JpsieeTagProbe()

    return Lists


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
