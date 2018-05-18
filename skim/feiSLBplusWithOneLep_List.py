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

from stdCharged import *


def BplusSLWithOneLep():
    applyCuts('B+:semileptonic', '-5<cosThetaBetweenParticleAndTrueB<3 and sigProb>0.009 and extraInfo(decayModeID)<8')
    # reconstruct Bsig to lepton

    stdE('95eff')
    stdMu('95eff')
    reconstructDecay('B+:sig1 -> e+:95eff', 'Mbc>0', 1)
    reconstructDecay('B+:sig2 -> mu+:95eff', 'Mbc>0', 2)
    reconstructDecay('B+:sig3 -> e-:95eff', 'Mbc>0', 3)
    reconstructDecay('B+:sig4 -> mu-:95eff', 'Mbc>0', 4)

    copyLists('B+:all', ['B+:sig1', 'B+:sig2', 'B+:sig3', 'B+:sig4'])

    reconstructDecay('Upsilon(4S):sig -> B-:semileptonic B+:all', '')
    applyCuts('B+:semileptonic', 'nParticlesInList(Upsilon(4S):sig)>0')

    BtagList = ['B+:semileptonic']
    return BtagList
