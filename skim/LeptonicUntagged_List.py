#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# SL skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *


def LeptonicList():
    cutAndCopyList('e-:highP', 'e-:all', 'useCMSFrame(p) > 2.0 and electronID > 0.5', True)
    cutAndCopyList('mu-:highP', 'mu-:all', 'useCMSFrame(p) > 2.0 and muonID > 0.5', True)
    reconstructDecay('B-:L0 -> e-:highP', '', 1)
    reconstructDecay('B-:L1 -> mu-:highP', '', 2)
    applyCuts('B-:L0', 'nTracks>4')
    applyCuts('B-:L1', 'nTracks>4')
    lepList = ['B-:L0', 'B-:L1']
    return lepList
