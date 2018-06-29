#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# Prepare all standard final state particles

# Note: hadron efficiencies in labels are calculated for 1<p<4 GeV/c


def stdPi(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standardized types of pion lists:
      - 'pi+:all' with no cuts on track
      - 'pi+:95eff' with 95% selection efficiency and good track
      - 'pi+:90eff' with 90% selection efficiency and good track
      - 'pi+:85eff' with 85% selection efficiency and good track

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('pi+:all', '', True, path=path)
    elif listtype == '95eff':
        fillParticleList('pi+:95eff', 'pionID > 0.002 and chiProb > 0.001', True, path=path)
    elif listtype == '90eff':
        fillParticleList('pi+:90eff', 'pionID > 0.075 and chiProb > 0.001', True, path=path)
    elif listtype == '85eff':
        fillParticleList('pi+:85eff', 'pionID > 0.275 and chiProb > 0.001', True, path=path)

###


def stdK(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standardized types of kaon lists:
      - 'K+:all' with no cuts on track
      - 'K+:95eff' with 95% selection efficiency and good track
      - 'K+:90eff' with 90% selection efficiency and good track
      - 'K+:85eff' with 85% selection efficiency and good track

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('K+:all', '', True, path=path)
    elif listtype == '95eff':
        fillParticleList('K+:95eff', 'kaonID > 0.002 and chiProb > 0.001', True, path=path)
    elif listtype == '90eff':
        fillParticleList('K+:90eff', 'kaonID > 0.043 and chiProb > 0.001', True, path=path)
    elif listtype == '85eff':
        fillParticleList('K+:85eff', 'kaonID > 0.218 and chiProb > 0.001', True, path=path)

###


def stdPr(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standardized types of proton lists:
      - 'p+:all' with no cuts on track
      - 'p+:90eff' with 90% selection efficiency and good track

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('p+:all', '', True, path=path)
    elif listtype == '90eff':
        fillParticleList('p+:90eff', 'protonID > 0.061 and chiProb > 0.001', True, path=path)

###


def stdE(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standardized types of electron lists:
      - 'e+:all' with no cuts on track
      - 'e+:95eff' with 95% selection efficiency and good track

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('e+:all', '', True, path=path)
    elif listtype == '95eff':
        fillParticleList('e+:95eff', 'electronID > 0.047 and chiProb > 0.001', True, path=path)

###


def stdMu(listtype='95eff', path=analysis_main):
    """
    Function to prepare one of several standardized types of muon lists:
      - 'mu+:all' with no cuts on track
      - 'mu+:95eff' with 95% selection efficiency and good track

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    if listtype == 'all':
        fillParticleList('mu+:all', '', True, path=path)
    elif listtype == '95eff':
        fillParticleList('mu+:95eff', 'muonID > 0.008 and chiProb > 0.001', True, path=path)

###


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
    stdK('all', path=path)
    stdPi('all', path=path)
    stdPr('all', path=path)
    stdE('all', path=path)
    stdMu('all', path=path)

    # Loose PID
    stdLooseK(path=path)
    stdLoosePi(path=path)
    stdLooseE(path=path)
    stdLooseMu(path=path)
    stdLoosePr(path=path)

###

# Loose FSParticles used for skimming


def stdLoosePi(path=analysis_main):
    """
    Creation of 'pi+:loose' list with the following requirements to the track:

      - 'pionID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('pi+:loose', 'pionID > 0.1 and chiProb > 0.001', True, path=path)


def stdLooseK(path=analysis_main):
    """
    Creation of 'K+:loose' list with the following requirements to the track:

      - 'kaonID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('K+:loose', 'kaonID > 0.1 and chiProb > 0.001', True, path=path)


def stdLooseMu(path=analysis_main):
    """
    Creation of 'mu+:loose' list with the following requirements to the track:

      - 'muonID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('mu+:loose', 'muonID > 0.1 and chiProb > 0.001', True, path=path)


def stdLooseE(path=analysis_main):
    """
    Creation of 'e+:loose' list with the following requirements to the track:

      - 'electronID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('e+:loose', 'electronID > 0.1 and chiProb > 0.001', True, path=path)


def stdLoosePr(path=analysis_main):
    """
    Creation of 'p+:loose' list with the following requirements to the track:

      - 'protonID > 0.1'
      - 'chiProb > 0.001'

    @param path     modules are added to this path
    """

    fillParticleList('p+:loose', 'protonID > 0.1 and chiProb > 0.001', True, path=path)
