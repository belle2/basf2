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


def B0SLWithOneLep():
    applyCuts('B0:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.01 and extraInfo(decayModeID)<8')
    # now signal side

    reconstructDecay('B0:sig1 -> e+:all', 'Mbc>0', 1)
    reconstructDecay('B0:sig2 -> mu+:all', 'Mbc>0', 2)
    reconstructDecay('B0:sig3 -> e-:all', 'Mbc>0', 3)
    reconstructDecay('B0:sig4 -> mu-:all', 'Mbc>0', 4)

    copyLists('B0:all', ['B0:sig1', 'B0:sig2', 'B0:sig3', 'B0:sig4'])

    reconstructDecay('Upsilon(4S):sig -> anti-B0:semileptonic B0:all', '')

    UpsilonList = ['Upsilon(4S):sig']
    return UpsilonList
