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

# If retention rate could be further reduced, can combine this with Jpsiee


def SystematicsList():

    Lists = []
    Lists += JpsimumuTagProbe()

    return Lists


def JpsimumuTagProbe():
    Cuts = '2.8 < M < 3.4'
    Channel = 'mu+:all mu-:loose'
    jpsiList = []
    chID = 0
    reconstructDecay('J/psi:mumutagprobe' + str(chID) + ' -> ' + Channel, Cuts, chID)
    jpsiList.append('J/psi:mumutagprobe' + str(chID))
    return jpsiList
