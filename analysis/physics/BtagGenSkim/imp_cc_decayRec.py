#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
#
# Reconstruction of the following ccbar mesons:
#
# o) J/psi
#  - 1: e- e+
#  - 2: mu- mu+
#
# o) psi(2S)
#  - 1: e- e+
#  - 2: mu- mu+:all pi+
#
# o) chi_cy
#  - 1: J/psi gamma   (y=0)
#  - 2: J/psi gamma   (y=1)
#  - 3: J/psi gamma   (y=2)
#
#
# Contributors: A. Zupanc (August 2014)
#               M. Lubej (December 2014)
#               A. Polsak (December 2014)
#
######################################################

from basf2 import *
from modularAnalysis import applyCuts
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import copyLists
from modularAnalysis import cutAndCopyList

import sys


def reconstructJpsi():
    # reconstruct J/psi decays
    reconstructDecay('J/psi:e -> e-:all e+:all', '3.05 < M < 3.15', 1)
    reconstructDecay('J/psi:mu -> mu-:all mu+:all', '3.05 < M < 3.15', 2)

    # merge individual lists into one list
    copyLists('J/psi:all', ['J/psi:e', 'J/psi:mu'])
    # perform MC matching (MC truth asociation)
    matchMCTruth('J/psi:all')
    # taking only correctly reconstructed
    applyCuts('J/psi:all', 'isSignal>0.5')


def reconstructPsi2S():
    # reconstruct decays
    reconstructDecay('psi(2S):e -> e-:all e+:all', '3.65 < M < 3.75', 1)
    reconstructDecay('psi(2S):mu -> mu-:all mu+:all', '3.65 < M < 3.75', 2)
    reconstructDecay('psi(2S):J/psi -> J/psi:all pi+:all pi-:all',
                     '3.65 < M < 3.75', 3)

    # merge individual lists into one list for
    copyLists('psi(2S):all', ['psi(2S):e', 'psi(2S):mu', 'psi(2S):J/psi'])
    # perform MC matching (MC truth asociation)
    matchMCTruth('psi(2S):all')
    # taking only correctly reconstructed
    applyCuts('psi(2S):all', 'isSignal>0.5')


def reconstructChi_cy():
    # reconstruct decays
    reconstructDecay('chi_c0:all -> J/psi:all gamma:all', '3.35 < M < 3.45', 1)
    reconstructDecay('chi_c1:all -> J/psi:all gamma:all', '3.45 < M < 3.55', 2)
    reconstructDecay('chi_c2:all -> J/psi:all gamma:all', '3.50 < M < 3.60', 3)

    # perform MC matching (MC truth asociation)
    matchMCTruth('chi_c0:all')
    matchMCTruth('chi_c1:all')
    matchMCTruth('chi_c2:all')

    # taking only correctly reconstructed D*+
    applyCuts('chi_c0:all', 'isSignal>0.5')
    applyCuts('chi_c1:all', 'isSignal>0.5')
    applyCuts('chi_c2:all', 'isSignal>0.5')


