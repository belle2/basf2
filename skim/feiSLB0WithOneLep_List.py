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

from stdCharged import *


def B0SLWithOneLep():
    applyCuts('B0:semileptonic', '-5<cosThetaBetweenParticleAndTrueB)<3 and sigProb>0.005 and extraInfo(decayModeID)<8')
    # now signal side

    stdE('95eff')
    stdMu('95eff')
    reconstructDecay('B0:sig1 -> e+:95eff', 'Mbc>0', 1)
    reconstructDecay('B0:sig2 -> mu+:95eff', 'Mbc>0', 2)
    reconstructDecay('B0:sig3 -> e-:95eff', 'Mbc>0', 3)
    reconstructDecay('B0:sig4 -> mu-:95eff', 'Mbc>0', 4)

    copyLists('B0:all', ['B0:sig1', 'B0:sig2', 'B0:sig3', 'B0:sig4'])

    reconstructDecay('Upsilon(4S):sig -> anti-B0:semileptonic B0:all', '')
    applyCuts('B0:semileptonic', 'nParticlesInList(Upsilon(4S):sig)>0')

    BtagList = ['B0:semileptonic']
    return BtagList
