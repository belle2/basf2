#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Btag skim Racha Cheain
#
######################################################
from basf2 import *
from modularAnalysis import *

from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')


def BplusHadronic():
    applyCuts('B+:generic', 'Mbc>5.24 and abs(deltaE)<0.200 and sigProb>0.001')

    BplushadronicList = ['B+:generic']
    return BplushadronicList
