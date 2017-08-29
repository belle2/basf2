#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Call to build all light mesons. Not recommended to use this general function as it creates many candidates


def loadStdLightMesons(persistent=True, path=analysis_main):

    loadStdLooseRho0(persistent, path)
    loadStdAllRho0(persistent, path)
    loadStdLooseRhoPlus(persistent, path)
    loadStdAllRhoPlus(persistent, path)
    loadStdLooseKstar(persistent, path)
    loadStdAllKstar(persistent, path)
    loadStdAllKstarPlus(persistent, path)
    loadStdLooseKstarPlus(persistent, path)

    loadStdLoosePhi(persistent, path)
    loadStdAllPhi(persistent, path)
    loadStdLooseF_0(persistent, path)
    loadStdAllF_0(persistent, path)
    loadStdLooseOmega(persistent, path)
    loadStdAllOmega(persistent, path)
    loadStdLooseEta(persistent, path)
    loadStdLooseEtaPrime(persistent, path)

# NoCut mesons not to be included by default


def loadStdLooseRho0(persistent=True, path=analysis_main):
    reconstructDecay('rho0:loose -> pi-:loose pi+:loose', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:loose'


def loadStdAllRho0(persistent=True, path=analysis_main):
    reconstructDecay('rho0:all -> pi-:all pi+:all', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:all'


def loadStdLooseRhoPlus(persistent=True, path=analysis_main):
    reconstructDecay('rho+:loose -> pi0:skim pi+:loose', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho+:loose'


def loadStdAllRhoPlus(persistent=True, path=analysis_main):
    reconstructDecay('rho+:all -> pi0:skim pi+:all', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho+:all'


def loadStdLooseKstar(persistent=True, path=analysis_main):
    reconstructDecay('K*0:loose -> pi-:loose K+:loose', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:loose'


def loadStdAllKstar(persistent=True, path=analysis_main):
    reconstructDecay('K*0:all -> pi-:all K+:all', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:all'


def loadStdLooseKstarPlus(persistent=True, path=analysis_main):
    reconstructDecay('K*+:loose ->K+:loose pi0:loose', '0.74 < M < 1.04 ', 1, persistent, path)
    return 'K*+:loose'


def loadStdAllKstarPlus(persistent=True, path=analysis_main):
    reconstructDecay('K*+:all -> K+:all pi0:skim', '0.74 < M <1.04', 1, persistent, path)
    return 'K*+:all'


def loadStdAllPhi(persistent=True, path=analysis_main):
    reconstructDecay('phi:all -> K+:all K-:all', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:all'


def loadStdLoosePhi(persistent=True, path=analysis_main):
    reconstructDecay('phi:loose -> K+:loose K-:loose', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:loose'


def loadStdAllF_0(persistent=True, path=analysis_main):
    reconstructDecay('f_0:all -> pi+:all pi-:all', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:all'


def loadStdLooseF_0(persistent=True, path=analysis_main):
    reconstructDecay('f_0:loose -> pi+:loose pi-:loose', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:loose'


def loadStdAllOmega(persistent=True, path=analysis_main):
    reconstructDecay('omega:all -> pi0:skim pi-:all pi+:all', '0.73 < M < 0.83', 1, persistent, path)
    return 'omega:all'


def loadStdLooseOmega(persistent=True, path=analysis_main):
    reconstructDecay('omega:loose -> pi0:skim pi-:loose pi+:loose', '0.73 < M < 0.83', 1, persistent, path)
    return 'omega:loose'


def loadStdLooseEta(persistent=True, path=analysis_main):
    reconstructDecay('eta:loose1 -> gamma:skim gamma:skim', '0.4 < M < 0.6', 1, persistent, path)
    reconstructDecay('eta:loose2 -> pi0:skim pi-:loose pi+:loose', '0.4 < M < 0.6', 2, persistent, path)
    copyLists('eta:loose', ['eta:loose1', 'eta:loose2'], persistent, path)
    return 'eta:loose'


def loadStdLooseEtaPrime(persistent=True, path=analysis_main):
    reconstructDecay('eta\':loose1 -> pi+:loose pi-:loose gamma:skim', '0.8 < M < 1.1', 1, persistent, path)
    reconstructDecay('eta\':loose2 -> pi+:loose pi-:loose eta:loose', '0.8 < M < 1.1', 2, persistent, path)
    copyLists('eta\':loose', ['eta\':loose1', 'eta\':loose2'], persistent, path)
    return 'eta\':loose'
