#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Prepare all standard final state particles

# Note: hadron efficiencies in labels are calculated for 1<p<4 GeV/c


def stdPi(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standartized types of pion lists:

      - 'pi+:all' with no cuts on track
      - 'pi+:99eff' with tracks satisfying 99% efficiency cuts and 'chiProb > 0.001'
      - 'pi+:95eff' (default) with tracks satisfying 95% efficiency cuts and 'chiProb > 0.001'
      - 'pi+:90eff' with tracks satisfying 90% efficiency cuts and 'chiProb > 0.001'
      - 'pi+:85eff' with tracks satisfying 85% efficiency cuts and 'chiProb > 0.001'

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('pi+:all', '', True, path)
    elif listtype == '99eff':
        fillParticleList('pi+:99eff', 'pionID > 0.009 and chiProb > 0.001', True, path)
    elif listtype == '95eff':
        fillParticleList('pi+:95eff', 'pionID > 0.046 and chiProb > 0.001', True, path)
    elif listtype == '90eff':
        fillParticleList('pi+:90eff', 'pionID > 0.426 and chiProb > 0.001', True, path)
    elif listtype == '85eff':
        fillParticleList('pi+:85eff', 'pionID > 0.658 and chiProb > 0.001', True, path)
    else:
        fillParticleList('pi+:all', '', True, path)

###


def stdK(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standartized types of kaon lists:

      - 'K+:all' with no cuts on track
      - 'K+:99eff' with tracks satisfying 99% efficiency cuts and 'chiProb > 0.001'
      - 'K+:95eff' (default) with tracks satisfying 95% efficiency cuts and 'chiProb > 0.001'
      - 'K+:90eff' with tracks satisfying 90% efficiency cuts and 'chiProb > 0.001'
      - 'K+:85eff' with tracks satisfying 85% efficiency cuts and 'chiProb > 0.001'

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('K+:all', '', True, path)
    elif listtype == '99eff':
        fillParticleList('K+:99eff', 'kaonID > 0.010 and chiProb > 0.001', True, path)
    elif listtype == '95eff':
        fillParticleList('K+:95eff', 'kaonID > 0.048 and chiProb > 0.001', True, path)
    elif listtype == '90eff':
        fillParticleList('K+:90eff', 'kaonID > 0.332 and chiProb > 0.001', True, path)
    elif listtype == '85eff':
        fillParticleList('K+:85eff', 'kaonID > 0.674 and chiProb > 0.001', True, path)
    else:
        fillParticleList('K+:all', '', True, path)

###


def stdPr(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standartized types of proton lists:

      - 'p+:all' with no cuts on track
      - 'p+:95eff' (default) with tracks satisfying cuts 'protonID > 0.079' and 'chiProb > 0.001'
      - 'p+:90eff' with tracks satisfying cuts 'protonID > 0.872' and chiProb > 0.001'

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('p+:all', '', True, path)
    elif listtype == '95eff':
        fillParticleList('p+:95eff', 'protonID > 0.079 and chiProb > 0.001', True, path)
    elif listtype == '90eff':
        fillParticleList('p+:90eff', 'protonID > 0.872 and chiProb > 0.001', True, path)
    else:
        fillParticleList('p+:all', '', True, path)

# Benchmarks for electrons are missing, default is 'all' instead of '95eff'


def stdE(listtype='all', path=analysis_main):
    """
    Function to prepare one of several standartized types of electron lists:
      - 'e+:all' with no cuts on track
      - 'e+:99eff' with 99% selection efficiency

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('e+:all', '', True, path)
    elif listtype == '99eff':
        fillParticleList('e+:99eff', 'electronID > 0.750 and chiProb > 0.001', True, path)
    else:
        fillParticleList('e+:all', '', True, path)

# Benchmarks for muons are missing, default is 'all' instead of '95eff'


def stdMu(listtype='all', path=analysis_main):
    """
    Function to prepare one of several standartized types of muon lists:
      - 'mu+:all' with no cuts on track
      - 'mu+:95eff' with 95% selection efficiency

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('mu+:all', '', True, path)
    elif listtype == '95eff':
        fillParticleList('mu+:95eff', 'muonID > 0.625 and chiProb > 0.001', True, path)
    else:
        fillParticleList('mu+:all', '', True, path)


def loadStdCharged(path=analysis_main):
    """
    Creating following lists of charged tracks:

      - 'pi+:all'
      - 'K+:all'
      - 'p+:all'
      - 'e+:all'
      - 'mu+:all'

    No requirements to tracks applied.

    @param path     modules are added to this path
    """

    # No PID
    stdK('all', path)
    stdPi('all', path)
    stdPr('all', path)
    stdE('all', path)
    stdMu('all', path)

    # Loose PID
    stdLooseK(path)
    stdLoosePi(path)
    stdLooseE(path)
    stdLooseMu(path)
    stdLoosePr(path)

###

# Loose FSParticles used for skimming


def stdLoosePi(path=analysis_main):
    """
    Creation of 'pi+:loose' list with the following requirements to the track:

      - 'pionID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('pi+:loose', 'pionID > 0.1 and chiProb > 0.001', True, path)


def stdLooseK(path=analysis_main):
    """
    Creation of 'K+:loose' list with the following requirements to the track:

      - 'kaonID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('K+:loose', 'kaonID > 0.1 and chiProb > 0.001', True, path)


def stdLooseMu(path=analysis_main):
    """
    Creation of 'mu+:loose' list with the following requirements to the track:

      - 'muonID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('mu+:loose', 'muonID > 0.1 and chiProb > 0.001', True, path)


def stdLooseE(path=analysis_main):
    """
    Creation of 'e+:loose' list with the following requirements to the track:

      - 'electronID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('e+:loose', 'electronID > 0.1 and chiProb > 0.001', True, path)


def stdLoosePr(path=analysis_main):
    """
    Creation of 'p+:loose' list with the following requirements to the track:

      - 'protonID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('p+:loose', 'protonID > 0.1 and chiProb > 0.001', True, path)
