#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import modularAnalysis as ma
from vertex import kFit
from stdPi0s import stdPi0s


def loadStdVeryLooseTracks(particletype, path):
    """
    Function to prepare veryLoose charged particle lists. We require only fiducial the cuts
    :b2:var:`thetaInCDCAcceptance` and :b2:var:`chiProb` :math:`> 0` and
    abs(:b2:var:`dr`) :math:`< 0.5~{\\rm cm}` and abs(dz) :math:` < 3~{\\rm cm}`.

    @param particletype type of charged particle to make a list of
    @param path         modules are added to this path
    """

    # basic quality cut strings
    trackQuality = 'thetaInCDCAcceptance and chiProb > 0 '
    ipCut = 'abs(dr) < 0.5 and abs(dz) < 3'
    goodTrack = trackQuality + ' and ' + ipCut

    if particletype not in ['pi', 'K', 'p', 'e', 'mu']:
        B2ERROR("The requested list is not a standard charged particle. Use one of pi, K, e, mu, p.")

    ma.fillParticleList(particletype + '+:veryLoose', goodTrack, True, path=path)


def loadStdVeryLooseKstar0(path):
    """
    Create a list of 'K*0:veryLoose' list from 'pi-:veryLoose K+:veryLoose' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*0:veryLoose -> K+:veryLoose pi-:veryLoose', '0.7 < M < 1.6', 1, path=path)
    return 'K*0:veryLoose'


def loadStdVeryLooseRho0(path):
    """
    Create a list of 'rho0:veryLoose' list from 'pi-:veryLoose pi+:veryLoose' with :math:`0.47 < M < 1.15~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho0:veryLoose -> pi+:veryLoose pi-:veryLoose', '0.47 < M < 1.15', 1, path=path)
    return 'rho0:veryLoose'


def loadStdVeryLooseRhoPlus(path):
    """
    Create a list of 'rho+:veryLoose' list from 'pi0:charmlessFit pi+:veryLoose' with :math:`0.47 < M < 1.15~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('rho+:veryLoose -> pi+:veryLoose pi0:charmlessFit', '0.47 < M < 1.15', 1, path=path)
    return 'rho+:veryLoose'


def loadStdPi0ForBToCharmless(path):
    """
    Creates a list 'pi0:charmlessFit' for :math:`B\\to {\\rm charmless}` skims, based on recommendations of
    the neutral group (avoiding ECL timing cuts) for winter 2020. We require the energy of the photons
    to be larger than :math:`22.5~{\\rm MeV}` in the forward end cap, :math:`20~{\\rm MeV}` in the barrel,
    and :math:`20~{\\rm MeV}` in the backward end cap. For the :math:`\\pi^{0}`, we require the mass to be
    :math:`105 < M < 150~{\\rm MeV}/c^2` and a massKFit to converge.
    """
    stdPi0s('all', path)
    ma.cutAndCopyList(outputListName='pi0:charmlessFit', inputListName='pi0:all',
                      cut='[[daughter(0,clusterReg)==1 and daughter(0,E)> 0.0225] or ' +
                      '[daughter(0,clusterReg)==2 and daughter(0,E)> 0.020] or ' +
                      '[daughter(0,clusterReg)==3 and daughter(0,E)> 0.020]] and ' +
                      '[[daughter(1,clusterReg)==1 and daughter(1,E)> 0.0225] or ' +
                      '[daughter(1,clusterReg)==2 and daughter(1,E)> 0.020] or ' +
                      '[daughter(1,clusterReg)==3 and daughter(1,E)> 0.020]] and ' +
                      'M > 0.105 and M < 0.150 ',
                      path=path)
    kFit('pi0:charmlessFit', 0.0, fit_type='vertex', path=path)


def loadStdVeryLooseKstarPlus(path):
    """
    Create a list of 'K*+:veryLoose' list from 'pi+:veryLoose K_S0:merged' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*+:veryLoose -> K_S0:merged pi+:veryLoose', '0.7 < M < 1.6', 1, path=path)
    return 'K*+:veryLoose'


def loadStdVeryLooseKstarPlusPi0(path):
    """
    Create a list of 'K*+:veryLoosePi0' list from 'K+:veryLoose pi0:charmlessFit' with :math:`0.74 < M < 1.04~GeV`

    @param persistent   whether RootOutput module should save the created ParticleLists (default True)
    @param path         modules are added to this path
    """
    ma.reconstructDecay('K*+:veryLoosePi0 -> K+:veryLoose pi0:charmlessFit', '0.7 < M < 1.6', 1, path=path)
    return 'K*+:veryLoosePi0'
