#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


def loadStdDiLeptons(persistent=True, path=analysis_main):
    """
    Create the following lists of di-leptons:
      - 'J/psi:mumuLoose'
      - 'J/psi:eeLoose'
      - 'psi(2S):mumuLoose'
      - 'psi(2S):eeLoose'

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """

    loadStdLooseJpsi2mumu(persistent, path)
    loadStdLooseJpsi2ee(persistent, path)
    loadStdLoosepsi2s2mumu(persistent, path)
    loadStdLoosepsi2s2ee(persistent, path)


def loadStdLooseJpsi2mumu(persistent=True, path=analysis_main):
    """
    Load the 'J/psi:mumuLoose' list from 'mu-:loose mu+:loose', with :math:`2.8 < M < 3.7~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('J/psi:mumuLoose -> mu-:loose mu+:loose', '2.8 < M < 3.7', 2, persistent, path)
    return 'J/psi:mumuLoose'


def loadStdLooseJpsi2ee(persistent=True, path=analysis_main):
    """
    Load the 'J/psi:eeLoose' list from 'e-:loose e+:loose', with :math:`2.8 < M < 3.7~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('J/psi:eeLoose -> e-:loose e+:loose', '2.8 < M < 3.7', 2, persistent, path)
    return 'J/psi:eeLoose'


def loadStdLoosepsi2s2mumu(persistent=True, path=analysis_main):
    """
    Load the 'psi(2S):mumuLoose' list from 'mu-:loose mu+:loose', with :math:`3.2 < M < 4.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('psi(2S):mumuLoose -> mu-:loose mu+:loose', '3.2 < M < 4.1', 2, persistent, path)
    return 'psi(2S):mumuLoose'


def loadStdLoosepsi2s2ee(persistent=True, path=analysis_main):
    """
    Load the 'psi(2S):eeLoose' list from 'e-:loose e+:loose', with :math:`3.2 < M < 4.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('psi(2S):eeLoose -> e-:loose e+:loose', '3.2 < M < 4.1', 2, persistent, path)
    return 'psi(2S):eeLoose'
