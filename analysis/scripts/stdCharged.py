#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *

# basic quality cut strings
trackQuality = 'thetaInCDCAcceptance and chiProb > 0.001'
ipCut = 'd0 < 0.5 and abs(z0) < 2'
goodTrack = trackQuality + ' and ' + ipCut
chargednames = ['pi', 'K', 'p', 'e', 'mu']
pidnames = ['pionID', 'kaonID', 'protonID', 'electronID', 'muonID']
effnames = ['95eff', '90eff', '85eff']
# efficiency cuts = [.95,.90,.85] efficiency; values outside (0,1) mean the cut does not exist and an error will be thrown
effcuts = [[0.002, 0.075, 0.275]
           [0.002, 0.043, 0.218]
           [0.000, 0.061, 1.000]
           [0.047, 1.000, 1.000]
           [0.008, 1.000, 1.000]]

# standard master function


def stdCharged(particletype, listtype, path=analysis_main):
    """
    Function to prepare one of several standardized types of charged particle lists:
      - 'all' with no cuts on track
      - 'good' high purity lists for data studies
      - 'loose' looses selection for skimming
      - 'higheff' high efficiency list with loose global ID cut for data studies
    Also the following lists, which may or may not be available depending on the release
      - '99eff' with 99% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)
      - '95eff' with 95% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)
      - '90eff' with 90% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)
      - '85eff' with 85% selection efficiency (calculated for 1<p<4 GeV) and good track (MC only)

    @param particletype type of charged particle to make a list of
    @param listtype     name of standard list
    @param path         modules are added to this path
    """

    if particletype not in chargednames:
        B2FATAL("The requested list is not a standard charged particle. Use one of pi, K, e, mu, p.")
    else:
        particleindex = chargednames.index(particletype)

    if listtype == 'all':
        fillParticleList(particletype + '+:all', '', True, path=path)
    elif listtype == 'good':
        fillParticleList(
            particletype + '+:good',
            PIDnames[particleindex] + ' > 0.5 and ' + goodTrack,
            True,
            path=path)
    elif listtype == 'loose':
        fillParticleList(
            particletype + '+:loose',
            PIDnames[particleindex] + ' > 0.1 and ' + goodTrack,
            True,
            path=path)
    elif listtype == 'higheff':
        fillParticleList(
            particletype + '+:higheff',
            PIDnames[particleindex] + ' > 0.002 and ' + goodTrack,
            True,
            path=path)
    elif listtype not in effnames:
        B2FATAL("The requested list is not defined. Please refer to the stdCharged documentation.")
    else:
        effindex = effnames.index(listtype)
        pidcut = effcuts[particleindex][effindex]
        if 0.0 < pidcut < 1.0:
            fillParticleList(
                particletype +
                '+:' +
                listtype,
                PIDnames[particleindex] +
                ' > ' +
                pidcut +
                ' and ' +
                goodTrack,
                True,
                path=path)
        else:
            B2FATAL('The requested standard particle list ' << particletype <<
                    '+:' << listtype << ' is not available in this release.')

###


def stdPi(listtype='good', path=analysis_main):
    stdCharged(particletype='pi', listtype, path)


def stdK(listtype='good', path=analysis_main):
    stdCharged(particletype='K', listtype, path)


def stdPr(listtype='good', path=analysis_main):
    stdCharged(particletype='p', listtype, path)


def stdE(listtype='good', path=analysis_main):
    stdCharged(particletype='e', listtype, path)


def stdMu(listtype='good', path=analysis_main):
    stdCharged(particletype='mu', listtype, path)

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
