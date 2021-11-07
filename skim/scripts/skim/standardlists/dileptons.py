#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma


def loadStdDiLeptons(persistent=True, path=None):
    """
    Create the following lists of di-leptons:
      - 'J/psi:mumuLoose'Psi2s
      - 'J/psi:eeLoose'
      - 'psi(2S):mumuLoose'
      - 'psi(2S):eeLoose'

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """

    loadStdLooseJpsi2mumu(persistent, path)
    loadStdLooseJpsi2ee(persistent, path)
    loadStdLooseJpsi2ee_noTOP(persistent, path)
    loadStdLoosePsi2s2mumu(persistent, path)
    loadStdLoosePsi2s2ee(persistent, path)
    loadStdPsi2s2lepton(persistent, path)


def loadStdLooseJpsi2mumu(persistent=True, path=None):
    """
    Load the 'J/psi:mumuLoose' list from 'mu-:loose mu+:loose', with :math:`2.8 < M < 3.7~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('J/psi:mumuLoose -> mu-:loose mu+:loose', '2.8 < M < 3.7', 2, persistent, path)
    return 'J/psi:mumuLoose'


def loadStdLooseJpsi2ee(persistent=True, path=None):
    """
    Load the 'J/psi:eeLoose' list from 'e-:loose e+:loose', with :math:`2.8 < M < 3.7~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('J/psi:eeLoose -> e-:loose e+:loose', '2.8 < M < 3.7', 2, persistent, path)
    return 'J/psi:eeLoose'


def loadStdJpsiToee(persistent=True, path=None):
    """
    Load the 'J/psi:ee' list from 'e+:bremCorr e-:bremCorr', with :math:`2.8 < M < 3.4~GeV` with electronID>0.01

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """

    ma.fillParticleList('e+:withCuts', cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=path)
    ma.fillParticleList(decayString='gamma:e+', cut="E < 1.0", path=path)
    ma.correctBrems(outputList='e+:bremCorr', inputList='e+:withCuts', gammaList='gamma:e+', multiplePhotons=False,
                    usePhotonOnlyOnce=True, writeOut=True, path=path)

    ma.reconstructDecay(
        'J/psi:ee -> e+:bremCorr e-:bremCorr',
        '2.8 < M < 3.4 and daughter(0, electronID) > 0.01 or daughter(1, electronID) > 0.01',
        2,
        persistent,
        path)
    return 'J/psi:ee'


def loadStdJpsiToee_noTOP(persistent=True, path=None):
    """
    Load the 'J/psi:ee' list from 'e+:bremCorr e-:bremCorr', with :math:`2.8 < M < 3.4~GeV` with electronID_noTOP>0.01

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """

    ma.fillParticleList('e+:withCuts', cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=path)
    ma.fillParticleList(decayString='gamma:e+', cut="E < 1.0", path=path)
    ma.correctBrems(outputList='e+:bremCorr', inputList='e+:withCuts', gammaList='gamma:e+', multiplePhotons=False,
                    usePhotonOnlyOnce=True, writeOut=True, path=path)

    ma.reconstructDecay(
        'J/psi:ee -> e+:bremCorr e-:bremCorr',
        '2.8 < M < 3.4 and daughter(0, electronID_noTOP) > 0.01 or daughter(1, electronID_noTOP) > 0.01',
        2,
        persistent,
        path)
    return 'J/psi:ee'


def loadStdJpsiTomumu(persistent=True, path=None):
    """
    Load the 'J/psi:mumu' list from 'mu+:withCuts mu+:withCuts', with :math:`2.8 < M < 3.4~GeV`
    where mu+:withCuts list is with cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and muonID > 0.01"

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.fillParticleList('mu+:withCuts', cut="dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance", path=path)
    ma.reconstructDecay(
        'J/psi:mumu -> mu+:withCuts mu-:withCuts',
        '2.8 < M < 3.4 and daughter(0, muonID) > 0.01 or daughter(1,muonID) > 0.01',
        2,
        persistent,
        path)
    return 'J/psi:mumu'


def loadStdPsi2s2lepton(persistent=True, path=None):
    """
    'psi(2S):ll' list of Psi(2S) to mumu or ee.
    'psi(2S):mumu' reconstructed from 'mu-:all mu+:all', with :math:`3.2 < M < 4.1~GeV`
    'psi(2S):ee' reconstructed from 'e-:all e+:all', with :math:`3.2 < M < 4.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('psi(2S):mumu -> mu-:all mu+:all', '3.2 < M < 4.1', 1, persistent, path=path)
    ma.reconstructDecay('psi(2S):ee -> e-:all e+:all', '3.2 < M < 4.1', 2, persistent, path=path)
    ma.copyLists('psi(2S):ll', ['psi(2S):mumu', 'psi(2S):ee'], persistent, path)
    return 'psi(2S):ll'


def loadStdLoosePsi2s2mumu(persistent=True, path=None):
    """
    Load the 'psi(2S):mumuLoose' list from 'mu-:loose mu+:loose', with :math:`3.2 < M < 4.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('psi(2S):mumuLoose -> mu-:loose mu+:loose', '3.2 < M < 4.1', 2, persistent, path)
    return 'psi(2S):mumuLoose'


def loadStdLoosePsi2s2ee(persistent=True, path=None):
    """
    Load the 'psi(2S):eeLoose' list from 'e-:loose e+:loose', with :math:`3.2 < M < 4.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('psi(2S):eeLoose -> e-:loose e+:loose', '3.2 < M < 4.1', 2, persistent, path)
    return 'psi(2S):eeLoose'
