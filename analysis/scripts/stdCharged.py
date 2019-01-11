#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *


# define arrays to interpret cut matrix
_chargednames = ['pi', 'K', 'p', 'e', 'mu']
_pidnames = ['pionID', 'kaonID', 'protonID', 'electronID', 'muonID']
_effnames = ['95eff', '90eff', '85eff']
# default particle list for stdPi() and similar functions
_defaultlist = 'good'


def _stdChargedEffCuts(particletype, listtype):
    """
    Provides the PID cut corresponding to a given efficiency percentile

    @param particletype  type of charged particle (pi, K, p, e, mu)
    @param listtype      efficiency percentile for the list (95eff, 90eff, 85eff)
    """

    particleindex = _chargednames.index(particletype)
    effindex = _effnames.index(listtype)

    # efficiency cuts = [.95,.90,.85] efficiency; values outside (0,1) mean the cut does not exist and an error will be thrown
    effcuts = [[0.002, 0.075, 0.275],
               [0.002, 0.043, 0.218],
               [0.000, 0.061, 1.000],
               [0.047, 1.000, 1.000],
               [0.008, 1.000, 1.000]]
    #
    return effcuts[particleindex][effindex]


def stdCharged(particletype, listtype, path=analysis_main):
    """
    Function to prepare one of several standardized types of charged particle lists:
      - 'all' with no cuts on track
      - 'good' high purity lists for data studies
      - 'loose' loose selections for skimming
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

    # basic quality cut strings
    trackQuality = 'thetaInCDCAcceptance and chiProb > 0.001'
    ipCut = 'd0 < 0.5 and abs(z0) < 2'
    goodTrack = trackQuality + ' and ' + ipCut

    if particletype not in _chargednames:
        B2ERROR("The requested list is not a standard charged particle. Use one of pi, K, e, mu, p.")

    if listtype == 'all':
        fillParticleList(particletype + '+:all', '', True, path=path)
    elif listtype == 'good':
        fillParticleList(
            particletype + '+:good',
            _pidnames[_chargednames.index(particletype)] + ' > 0.5 and ' + goodTrack,
            True,
            path=path)
    elif listtype == 'loose':
        fillParticleList(
            particletype + '+:loose',
            _pidnames[_chargednames.index(particletype)] + ' > 0.1 and ' + goodTrack,
            True,
            path=path)
    elif listtype == 'higheff':
        fillParticleList(
            particletype + '+:higheff',
            _pidnames[_chargednames.index(particletype)] + ' > 0.002 and ' + goodTrack,
            True,
            path=path)
    elif listtype not in _effnames:
        B2ERROR("The requested list is not defined. Please refer to the stdCharged documentation.")
    else:
        pidcut = _stdChargedEffCuts(particletype, listtype)
        if 0.0 < pidcut < 1.0:
            fillParticleList(
                particletype +
                '+:' +
                listtype,
                _pidnames[_chargednames.index(particletype)] +
                ' > ' +
                str(pidcut) +
                ' and ' +
                goodTrack,
                True,
                path=path)
        else:
            B2ERROR('The requested standard particle list ' + particletype +
                    '+:' + listtype + ' is not available in this release.')

###


def stdPi(listtype=_defaultlist, path=analysis_main):
    """
    Function to prepare standard pion lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('pi', listtype, path)


def stdK(listtype=_defaultlist, path=analysis_main):
    """
    Function to prepare standard kaon lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('K', listtype, path)


def stdPr(listtype=_defaultlist, path=analysis_main):
    """
    Function to prepare standard proton lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('p', listtype, path)


def stdE(listtype=_defaultlist, path=analysis_main):
    """
    Function to prepare standard electron lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('e', listtype, path)


def stdMu(listtype=_defaultlist, path=analysis_main):
    """
    Function to prepare standard muon lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('mu', listtype, path)
