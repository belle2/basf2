#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma
from vertex import kFit
from stdPi0s import stdPi0s

# Creates list of neutral pions for hadronic B skims following the recommendations of the neutral group
# for winter 2020.


def loadStdPi0ForBToHadrons(persistent=True, path=None):
    """
    Creates a list 'pi0:bth_skim' for :math:`B\\to {\\rm hadrons}` skims, based on recommendations of
    the neutral group (avoiding ECL timing cuts) for winter 2020 and BtoCharmless for 2021. We require the energy of the photons
    to be larger than :math:`22.50~{\\rm MeV}` in the forward end cap, :math:`20~{\\rm MeV}` in the barrel,
    and :math:`20~{\\rm MeV}` in the backward end cap. For the :math:`\\pi^{0}`, we require the mass to be
    :math:`105 < M < 150~{\\rm MeV}/c^2` and a mass-constrained KFit to converge.
    """
    stdPi0s('all', path)
    ma.cutAndCopyList(outputListName='pi0:bth_skim', inputListName='pi0:all',
                      cut='[[daughter(0, clusterReg) == 1 and daughter(0, E) > 0.02250] or ' +
                      '[daughter(0, clusterReg) == 2 and daughter(0, E) > 0.020] or ' +
                      '[daughter(0, clusterReg) == 3 and daughter(0, E) > 0.020]] and ' +
                      '[[daughter(1, clusterReg) == 1 and daughter(1, E) > 0.02250] or ' +
                      '[daughter(1, clusterReg) == 2 and daughter(1, E) > 0.020] or ' +
                      '[daughter(1, clusterReg) == 3 and daughter(1, E) > 0.020]] and ' +
                      'M > 0.105 and M < 0.150', path=path)
    kFit('pi0:bth_skim', 0.0, 'mass', path=path)


def loadStdSkimHighEffTracks(particletype, path):
    """
    Function to prepare high eff charged particle lists (:SkimHighEff).
    We require only the fiducial cuts
    :b2:var:`thetaInCDCAcceptance`, :b2:var:`chiProb` :math:`> 0`,
    abs(:b2:var:`dr`) :math:`< 0.5~{\\rm cm}` and abs(dz) :math:` < 3~{\\rm cm}`,
    and (global) PID>0.01

    @param particletype type of charged particle to make a list of
    @param path         modules are added to this path
    """

    pidnames = {'pi': 'binaryPID(211,321)', 'K': 'binaryPID(321,211)', 'p': 'protonID', 'e': 'electronID', 'mu': 'muonID'}

    # basic quality cut strings
    trackQuality = 'thetaInCDCAcceptance and chiProb > 0 '
    ipCut = 'abs(dr) < 0.5 and abs(dz) < 3'
    goodTrack = trackQuality + ' and ' + ipCut

    if particletype not in pidnames.keys():
        ma.B2ERROR(f"The requested list is not a standard charged particle. Use one of {pidnames.keys()}")
    pidCut = f'{pidnames[particletype]} > 0.01 and {goodTrack}'

    ma.fillParticleList(particletype + '+:SkimHighEff', pidCut, True, path=path)

# Call to build all light mesons. Not recommended to use this general function as it creates many candidates


def loadStdLightMesons(persistent=True, path=None):
    """
    Create the following lists of light mesons:
      - 'rho0:loose'
      - 'rho0:all'
      - 'rho+:loose'
      - 'K*0:loose'
      - 'K*0:all'
      - 'K*+:loose'
      - 'phi:loose'
      - 'phi:all'
      - 'f_0:loose'
      - 'f_0:all'
      - 'omega:loose'
      - 'omega:all'
      - 'eta:loose'
      - 'eta\':loose'
      - 'eta:all'
      - 'eta\':all'

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """

    loadStdLooseRho0(persistent, path)
    loadStdAllRho0(persistent, path)
    loadStdLooseRhoPlus(persistent, path)
    loadStdLooseKstar0(persistent, path)
    loadStdAllKstar0(persistent, path)
    loadStdLooseKstarPlus(persistent, path)
    loadStdLoosePhi(persistent, path)
    loadStdAllPhi(persistent, path)
    loadStdLooseF_0(persistent, path)
    loadStdAllF_0(persistent, path)
    loadStdLooseOmega(persistent, path)
    loadStdAllOmega(persistent, path)
    loadStdLooseEta(persistent, path)
    loadStdAllEta(persistent, path)
    loadStdLooseEtaPrime(persistent, path)
    loadStdAllEtaPrime(persistent, path)

# NoCut mesons not to be included by default


def loadStdLooseRho0(persistent=True, path=None):
    """
    Create a list of 'rho0:loose' list from 'pi-:loose pi+:loose' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho0:loose -> pi-:loose pi+:loose', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:loose'


def loadStdAllRho0(persistent=True, path=None):
    """
    Create a list of 'rho0:all' list from 'pi-:all pi+:all' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho0:all -> pi-:all pi+:all', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:all'


def loadStdSkimHighEffRho0(persistent=True, path=None):
    """
    Create a list of 'rho0:SkimHighEff' list from 'pi-:SkimHighEff pi+:SkimHighEff' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho0:SkimHighEff -> pi-:SkimHighEff pi+:SkimHighEff', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho0:SkimHighEff'


def loadStdLooseRhoPlus(persistent=True, path=None):
    """
    Create a list of 'rho+:loose' list from 'pi0:eff40_May2020 pi+:loose' with :math:`0.47 < M < 1.07~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho+:loose -> pi+:loose pi0:eff40_May2020', '0.47 < M < 1.07', 1, persistent, path)
    return 'rho+:loose'


def loadStdAllRhoPlus(persistent=True, path=None):
    """
    Create a list of 'rho+:all' list from 'pi0:bth_skim' pi+:all' with :math:`0.47 < M < 1.07~GeV`.
    We apply few sanity cuts on the pi+: thetaInCDCAcceptance, abs(dr) < 0.5, and abs(dz) < 3.

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho+:all -> pi+:all pi0:bth_skim ', '0.47 < M < 1.07 and ' +
                        'daughter(0,thetaInCDCAcceptance) > 0 and abs(daughter(0,dr)) < 0.5 and ' +
                        'abs(daughter(0,dz)) < 3', 1, persistent, path)
    return 'rho+:all'


def loadA_1Plus(persistent=True, path=None):
    """
    Creates a 'a_1+:all' list from 'pi+:all pi+:all pi-:all' requiring :math:`0.8 < M < 1.6~{\\rm GeV}/c^2`.

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay(
        decayString='a_1+:all -> pi+:all pi+:all pi-:all',
        cut='0.8 < M < 1.6',
        dmID=1, writeOut=persistent,
        path=path)

    return 'a_1+:all'


def loadStdLooseKstar0(persistent=True, path=None):
    """
    Create a list of 'K*0:loose' list from 'pi-:loose K+:loose' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*0:loose -> pi-:loose K+:loose', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:loose'


def loadStdAllKstar0(persistent=True, path=None):
    """
    Create a list of 'K*0:all' list from 'pi-:all K+:all' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*0:all -> pi-:all K+:all', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:all'


def loadStdSkimHighEffKstar0(persistent=True, path=None):
    """
    Create a list of 'K*0:SkimHighEff' list from 'pi-:SkimHighEff K+:SkimHighEff' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*0:SkimHighEff -> pi-:SkimHighEff K+:SkimHighEff', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*0:SkimHighEff'


def loadStdLooseKstarPlus(persistent=True, path=None):
    """
    Create a list of 'K*+:loose' list from 'pi+:loose K_S0:merged' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*+:loose -> pi+:loose K_S0:merged', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*+:loose'


def loadStdAllKstarPlus(persistent=True, path=None):
    """
    Create a list of 'K*+:all' list from 'pi+:all K_S0:merged' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*+:all -> pi+:all K_S0:merged', '0.74 < M < 1.04', 1, persistent, path)
    return 'K*+:all'


def loadStdAllPhi(persistent=True, path=None):
    """
    Create a list of 'phi:all' list from 'K+:all K-:all' with :math:`0.97 < M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('phi:all -> K+:all K-:all', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:all'


def loadStdSkimHighEffPhi(persistent=True, path=None):
    """
    Create a list of 'phi:SkimHighEff' list from 'K+:SkimHighEff K-:SkimHighEff' with :math:`0.97 < M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('phi:SkimHighEff -> K+:SkimHighEff K-:SkimHighEff', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:SkimHighEff'


def loadStdLoosePhi(persistent=True, path=None):
    """
    Create a list of 'phi:loose' list from 'K+:loose K-:loose' with :math:`0.97 < M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('phi:loose -> K+:loose K-:loose', '0.97 < M < 1.1', 1, persistent, path)
    return 'phi:loose'


def loadStdAllF_0(persistent=True, path=None):
    """
    Create a list of 'f_0:all' list from 'pi+:all pi-:all' with :math:`0.78 < M < 1.18~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('f_0:all -> pi+:all pi-:all', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:all'


def loadStdSkimHighEffF_0(persistent=True, path=None):
    """
    Create a list of 'f_0:SkimHighEff' list from 'pi+:SkimHighEff pi-:SkimHighEff' with :math:`0.78 < M < 1.18~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('f_0:SkimHighEff -> pi+:SkimHighEff pi-:SkimHighEff', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:SkimHighEff'


def loadStdLooseF_0(persistent=True, path=None):
    """
    Create a list of 'f_0:loose' list from 'pi+:loose pi1:loose' with :math:`0.78 < M < 1.18~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('f_0:loose -> pi+:loose pi-:loose', '0.78 < M < 1.18', 1, persistent, path)
    return 'f_0:loose'


def loadStdAllOmega(persistent=True, path=None):
    """
    Create a list of 'omega:all' list from 'pi0:eff40_May2020 pi-:all pi+:all' with :math:`0.73 < M < 0.83~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('omega:all -> pi0:eff40_May2020 pi-:all pi+:all', '0.73 < M < 0.83', 1, persistent, path)
    return 'omega:all'


def loadStdSkimHighEffOmega(persistent=True, path=None):
    """
    Create a list of 'omega:SkimHighEff' list from 'pi0:eff40_May2020 pi-:SkimHighEff pi+:SkimHighEff'
    with :math:`0.73 < M < 0.83~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay(
        'omega:SkimHighEff -> pi0:eff40_May2020 pi-:SkimHighEff pi+:SkimHighEff',
        '0.73 < M < 0.83',
        1,
        persistent,
        path)
    return 'omega:SkimHighEff'


def loadStdLooseOmega(persistent=True, path=None):
    """
    Create a list of 'omega:loose' list from 'pi0:eff40_May2020 pi-:loose pi+:loose' with :math:`0.73 < M < 0.83~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('omega:loose -> pi0:eff40_May2020 pi-:loose pi+:loose', '0.73 < M < 0.83', 1, persistent, path)
    return 'omega:loose'


def loadStdWideOmega(persistent=True, path=None):
    """
    Create the list 'omega:wide' of omega mesons within the wide mass range :math:`0.71 < M < 0.85~GeV`
    from two oppositely charged pions that come from close to the IP, are inside the CDC acceptance,
    and fulfill a loose PID criterion, and a neutral pion from the 'pi0:skim' list, which isn't
    slow :math:`p > 0.25~GeV` and whose mass is above :math:`0.11~GeV`.

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.fillParticleList('pi+:wideOmega', 'dr < 1 and abs(dz) < 3 and thetaInCDCAcceptance and pionID > 0.1', path=path)
    ma.cutAndCopyList('pi0:wideOmega', 'pi0:skim', 'p > 0.25 and 0.11 < InvM < 0.15', path=path)
    ma.reconstructDecay('omega:wide -> pi+:wideOmega pi-:wideOmega pi0:wideOmega', '0.71 < M < 0.85',
                        writeOut=persistent, path=path)
    return 'omega:wide'


def loadStdAllEta(persistent=True, path=None):
    """
    Create a list of 'eta:all' list from 'gamma:all gamma:all' (dmID=1) and 'pi0:eff40_May2020 pi-:all pi+:all'
    (dmID=2), with :math:`0.4< M < 0.6~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('eta:all1 -> gamma:all gamma:all', '0.4 < M < 0.6', 1, persistent, path)
    ma.reconstructDecay('eta:all2 -> pi0:eff40_May2020 pi-:all pi+:all', '0.4 < M < 0.6', 2, persistent, path)
    ma.copyLists('eta:all', ['eta:all1', 'eta:all2'], persistent, path)
    return 'eta:all'


def loadStdPhotonCutEta(persistent=True, path=None):
    """
    Create the list 'eta:gm' of eta mesons with :math:`0.35 < M < 0.7~GeV` from two photons with minimal energy of :math:`50~MeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    etacuts = "0.35 < M < 0.7 and daughterLowest(E) > 0.05"
    ma.reconstructDecay('eta:gm -> gamma:all gamma:all', etacuts, 1, persistent, path)
    return 'eta:gm'


def loadStdSkimHighEffEta(persistent=True, path=None):
    """
    Create a list of 'eta:SkimHighEff' list from 'gamma:tight gamma:tight' (dmID=1) and
    'pi0:eff40_May2020 pi-:SkimHighEff pi+:SkimHighEff'
    (dmID=2), with :math:`0.4< M < 0.6~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('eta:SkimHighEff1 -> gamma:tight gamma:tight', '0.4 < M < 0.6', 1, persistent, path)
    ma.reconstructDecay(
        'eta:SkimHighEff2 -> pi0:eff40_May2020 pi-:SkimHighEff pi+:SkimHighEff',
        '0.4 < M < 0.6',
        2,
        persistent,
        path)
    ma.copyLists('eta:SkimHighEff', ['eta:SkimHighEff1', 'eta:SkimHighEff2'], persistent, path)
    return 'eta:SkimHighEff'


def loadStdLooseEta(persistent=True, path=None):
    """
    Create a list of 'eta:loose' list from 'gamma:loose gamma:loose' (dmID=1) and 'pi0:eff40_May2020 pi-:loose pi+:loose'
    (dmID=2), with :math:`0.4< M < 0.6~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('eta:loose1 -> gamma:loose gamma:loose', '0.4 < M < 0.6', 1, persistent, path)
    ma.reconstructDecay('eta:loose2 -> pi0:eff40_May2020 pi-:loose pi+:loose', '0.4 < M < 0.6', 2, persistent, path)
    ma.copyLists('eta:loose', ['eta:loose1', 'eta:loose2'], persistent, path)
    return 'eta:loose'


def loadStdAllEtaPrime(persistent=True, path=None):
    """
    Create a list of 'eta\':all' list from 'pi+:all pi-:all gamma:all' (dmID=1) and 'pi+:all pi-:all eta:all'
    (dmID=2), with :math:`0.8< M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('eta\':all1 -> pi+:all pi-:all gamma:all', '0.8 < M < 1.1', 1, persistent, path)
    ma.reconstructDecay('eta\':all2 -> pi+:all pi-:all eta:all', '0.8 < M < 1.1', 2, persistent, path)
    ma.copyLists('eta\':all', ['eta\':all1', 'eta\':all2'], persistent, path)
    return 'eta\':all'


def loadStdSkimHighEffEtaPrime(persistent=True, path=None):
    """
    Create a list of 'eta\':SkimHighEff' list from 'pi+:SkimHighEff pi-:SkimHighEff gamma:tight' (dmID=1)
    and 'pi+:SkimHighEff pi-:SkimHighEff eta:SkimHighEff'
    (dmID=2), with :math:`0.8< M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('eta\':SkimHighEff1 -> pi+:SkimHighEff pi-:SkimHighEff gamma:tight', '0.8 < M < 1.1', 1, persistent, path)
    ma.reconstructDecay(
        'eta\':SkimHighEff2 -> pi+:SkimHighEff pi-:SkimHighEff eta:SkimHighEff',
        '0.8 < M < 1.1',
        2,
        persistent,
        path)
    ma.copyLists('eta\':SkimHighEff', ['eta\':SkimHighEff1', 'eta\':SkimHighEff2'], persistent, path)
    return 'eta\':SkimHighEff'


def loadStdLooseEtaPrime(persistent=True, path=None):
    """
    Create a list of 'eta\':loose' list from 'pi+:loose pi-:loose gamma:loose' (dmID=1) and 'pi+:loose pi-:loose eta:loose'
    (dmID=2), with :math:`0.8< M < 1.1~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.cutAndCopyList('gamma:etaprg', 'gamma:loose', 'E>0.1')
    ma.reconstructDecay('eta\':loose1 -> pi+:loose pi-:loose gamma:etaprg', '0.8 < M < 1.1', 1, persistent, path)
    ma.reconstructDecay('eta\':loose2 -> pi+:loose pi-:loose eta:loose', '0.8 < M < 1.1', 2, persistent, path)
    ma.copyLists('eta\':loose', ['eta\':loose1', 'eta\':loose2'], persistent, path)
    return 'eta\':loose'
