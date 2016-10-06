#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def loadStdDiLeptons(persistent=True, path=analysis_main):

    loadStdLooseJpsi2mumu(persistent, path)
    loadStdLooseJpsi2ee(persistent, path)
    loadStdLoosepsi2s2mumu(persistent, path)
    loadStdLoosepsi2s2ee(persistent, path)


def loadStdLooseJpsi2mumu(persistent=True, path=analysis_main):
    reconstructDecay('J/psi:mumuLoose -> mu-:loose mu+:loose', '2.8 < M < 3.7', 2, persistent, path)
    return 'J/psi:mumuLoose'


def loadStdLooseJpsi2ee(persistent=True, path=analysis_main):
    reconstructDecay('J/psi:eeLoose -> e-:loose e+:loose', '2.8 < M < 3.7', 2, persistent, path)
    return 'J/psi:eeLoose'


def loadStdLoosepsi2s2mumu(persistent=True, path=analysis_main):
    reconstructDecay('psi(2S):mumuLoose -> mu-:loose mu+:loose', '3.2 < M < 4.1', 2, persistent, path)
    return 'psi(2S):mumuLoose'


def loadStdLoosepsi2s2ee(persistent=True, path=analysis_main):
    reconstructDecay('psi(2S):eeLoose -> e-:loose e+:loose', '3.2 < M < 4.1', 2, persistent, path)
    return 'psi(2S):eeLoose'
