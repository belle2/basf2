#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# FEI SL Skim + one lepton on the Bsig side
# Racha Cheaib
######################################################
from basf2 import *
from modularAnalysis import *

from variables import *
variables.addAlias('sigProb', 'extraInfo(SignalProbability)')


def BplusSLWithOneLep():
    applyCuts('B+:semileptonic', 'abs(cosThetaBetweenParticleAndTrueB)<10 and sigProb>0.01 and extraInfo(decayModeID)<8')
    # reconstruct Bsig to lepton
    reconstructDecay('B+:sig1 -> e+:all', 'Mbc>0', 1)
    reconstructDecay('B+:sig2 -> mu+:all', 'Mbc>0', 2)
    reconstructDecay('B+:sig3 -> e-:all', 'Mbc>0', 3)
    reconstructDecay('B+:sig4 -> mu-:all', 'Mbc>0', 4)

    copyLists('B+:all', ['B+:sig1', 'B+:sig2', 'B+:sig3', 'B+:sig4'])

    reconstructDecay('Upsilon(4S):sig -> B-:semileptonic B+:all', '')

    UpsilonList = ['Upsilon(4S):sig']
    return UpsilonList
