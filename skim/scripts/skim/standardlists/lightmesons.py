#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Call to build all light mesons. Not recommended to use this general function as it creates many candidates


def loadStdLightMesons(persistent=True, path=analysis_main):
    """
    Create the following lists of light mesons:
      - 'rho0:loose'
      - 'rho+:loose'
      - 'K*0:loose'
      - 'K*+:loose'
      - 'phi:loose'
      - 'f_0:loose'
      - 'omega:loose'
      - 'eta:loose'
      - 'eta\':loose'

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """

    loadStdLooseRho0(persistent, path)
    loadStdLooseRhoPlus(persistent, path)
    loadStdLooseKstar0(persistent, path)
    loadStdLooseKstarPlus(persistent, path)
    loadStdLoosePhi(persistent, path)
    loadStdLooseF_0(persistent, path)
    loadStdLooseOmega(persistent, path)
    loadStdLooseEta(persistent, path)
    loadStdLooseEtaPrime(persistent, path)

# NoCut mesons not to be included by default


def loadStdLooseRho0(persistent=True, path=analysis_main):
    """
    Create a list of 'rho0:loose' list from 'pi-:loose pi+:loose' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('rho0:loose -> pi-:loose pi+:loose', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:loose'


def loadStdAllRho0(persistent=True, path=analysis_main):
    """
    Create a list of 'rho0:all' list from 'pi-:all pi+:all' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('rho0:all -> pi-:all pi+:all', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:all'


def loadStdLooseRhoPlus(persistent=True, path=analysis_main):
    """
    Create a list of 'rho+:loose' list from 'pi0:loose pi+:loose' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('rho+:loose -> pi0:loose pi+:loose', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho+:loose'


def loadStdAllRhoPlus(persistent=True, path=analysis_main):
    """
    Create a list of 'rho+:all' list from 'pi0:loose pi+:all' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('rho+:all -> pi0:loose pi+:all', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho+:all'


def loadStdLooseKstar0(persistent=True, path=analysis_main):
    """
    Create a list of 'K*0:loose' list from 'pi-:loose K+:loose' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('K*0:loose -> pi-:loose K+:loose', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:loose'


def loadStdAllKstar0(persistent=True, path=analysis_main):
    """
    Create a list of 'K*0:all' list from 'pi-:all K+:all' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('K*0:all -> pi-:all K+:all', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:all'


def loadStdLooseKstarPlus(persistent=True, path=analysis_main):
    """
    Create a list of 'K*+:loose' list from 'pi+:loose K_S0:all' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('K*+:loose -> pi+:loose K_S0:all', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*+:loose'


def loadStdAllKstarPlus(persistent=True, path=analysis_main):
    """
    Create a list of 'K*+:all' list from 'pi+:all K_S0:all' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('K*+:all -> pi+:all K_S0:all', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*+:all'


def loadStdAllPhi(persistent=True, path=analysis_main):
    """
    Create a list of 'phi:all' list from 'K+:all K-:all' with :math:`0.97 < M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('phi:all -> K+:all K-:all', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:all'


def loadStdLoosePhi(persistent=True, path=analysis_main):
    """
    Create a list of 'phi:loose' list from 'K+:loose K-:loose' with :math:`0.97 < M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('phi:loose -> K+:loose K-:loose', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:loose'


def loadStdAllF_0(persistent=True, path=analysis_main):
    """
    Create a list of 'f_0:all' list from 'pi+:all pi-:all' with :math:`0.78 < M < 1.18~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('f_0:all -> pi+:all pi-:all', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:all'


def loadStdLooseF_0(persistent=True, path=analysis_main):
    """
    Create a list of 'f_0:loose' list from 'pi+:loose pi1:loose' with :math:`0.78 < M < 1.18~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('f_0:loose -> pi+:loose pi-:loose', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:loose'


def loadStdAllOmega(persistent=True, path=analysis_main):
    """
    Create a list of 'omega:all' list from 'pi0:loose pi-:all pi+:all' with :math:`0.73 < M < 0.83~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('omega:all -> pi0:loose pi-:all pi+:all', '0.73 < M < 0.83', 1, persistent, path)
    return 'omega:all'


def loadStdLooseOmega(persistent=True, path=analysis_main):
    """
    Create a list of 'omega:loose' list from 'pi0:loose pi-:loose pi+:loose' with :math:`0.73 < M < 0.83~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('omega:loose -> pi0:loose pi-:loose pi+:loose', '0.73 < M < 0.83', 1, persistent, path)
    return 'omega:loose'


def loadStdLooseEta(persistent=True, path=analysis_main):
    """
    Create a list of 'eta:loose' list from 'gamma:loose gamma:loose' (dmID=1) and 'pi0:loose pi-:loose pi+:loose'
    (dmID=2), with :math:`0.4< M < 0.6~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('eta:loose1 -> gamma:loose gamma:loose', '0.4 < M < 0.6', 1, persistent, path)
    reconstructDecay('eta:loose2 -> pi0:loose pi-:loose pi+:loose', '0.4 < M < 0.6', 2, persistent, path)
    copyLists('eta:loose', ['eta:loose1', 'eta:loose2'], persistent, path)
    return 'eta:loose'


def loadStdLooseEtaPrime(persistent=True, path=analysis_main):
    """
    Create a list of 'eta\':loose' list from 'pi+:loose pi-:loose gamma:loose' (dmID=1) and 'pi+:loose pi-:loose eta:loose'
    (dmID=2), with :math:`0.8< M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    reconstructDecay('eta\':loose1 -> pi+:loose pi-:loose gamma:loose', '0.8 < M < 1.1', 1, persistent, path)
    reconstructDecay('eta\':loose2 -> pi+:loose pi-:loose eta:loose', '0.8 < M < 1.1', 2, persistent, path)
    copyLists('eta\':loose', ['eta\':loose1', 'eta\':loose2'], persistent, path)
    return 'eta\':loose'
