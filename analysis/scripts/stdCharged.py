#!/usr/bin/env python3

from basf2 import B2ERROR
from modularAnalysis import fillParticleList


# define arrays to interpret cut matrix
_chargednames = ['pi', 'K', 'p', 'e', 'mu']
_pidnames = ['pionID', 'kaonID', 'protonID', 'electronID', 'muonID']
_effnames = ['95eff', '90eff', '85eff']
# default particle list for stdPi() and similar functions
_defaultlist = 'good'
_mostLikelyList = 'mostlikely'


def _stdChargedEffCuts(particletype, listtype):
    """
    Provides the PID cut corresponding to a given efficiency percentile

    @param particletype  type of charged particle (pi, K, p, e, mu)
    @param listtype      efficiency percentile for the list (95eff, 90eff, 85eff)
    """

    particleindex = _chargednames.index(particletype)
    effindex = _effnames.index(listtype)

    # efficiency cuts = [.95,.90,.85] efficiency; values outside (0,1) mean the cut does not exist and an error will be thrown
    effcuts = [[0.001, 0.019, 0.098],
               [5e-6,  0.027, 0.167],
               [0.000, 0.043, 0.251],
               [0.093, 0.301, 0.709],
               [0.187, 0.418, 0.909]]
    #
    return effcuts[particleindex][effindex]


def stdCharged(particletype, listtype, path):
    """
    Function to prepare one of several standardized types of charged particle lists:
      - 'all' with no cuts on track
      - 'good' high purity lists for data studies
      - 'loosepid' loose selections for skimming, PID cut only
      - 'loose' loose selections for skimming
      - 'higheff' high efficiency list with loose global ID cut for data studies
      - 'mostlikely' list with the highest PID likelihood
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
    trackQuality = 'thetaInCDCAcceptance and nCDCHits>20'
    ipCut = 'dr < 0.5 and abs(dz) < 2'
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
    elif listtype == 'loosepid':
        fillParticleList(
            particletype + '+:loosepid',
            _pidnames[_chargednames.index(particletype)] + ' > 0.1',
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


def stdPi(listtype=_defaultlist, path=None):
    """
    Function to prepare standard pion lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('pi', listtype, path)


def stdK(listtype=_defaultlist, path=None):
    """
    Function to prepare standard kaon lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('K', listtype, path)


def stdPr(listtype=_defaultlist, path=None):
    """
    Function to prepare standard proton lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('p', listtype, path)


def stdE(listtype=_defaultlist, path=None):
    """
    Function to prepare standard electron lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('e', listtype, path)


def stdMu(listtype=_defaultlist, path=None):
    """
    Function to prepare standard muon lists, refer to stdCharged for details

    @param listtype     name of standard list
    @param path         modules are added to this path
    """
    stdCharged('mu', listtype, path)


def stdMostLikely(pidPriors=None, suffix='', custom_cuts='', path=None):
    """
    Function to prepare most likely particle lists according to PID likelihood, refer to stdCharged for details

    @param pidPriors    list of 6 float numbers used to reweight PID likelihoods
    @param suffix       string added to the end of particle list names
    @param custom_cuts  custom selection cut string, if empty, standard track quality cuts will be applied
    @param path         modules are added to this path
    """
    # Here we need basic track quality cuts to be applied,
    # otherwise, we get a lot of badly reconstructed particles,
    # which will end up filled as a random type
    args = ''
    if pidPriors is not None:
        args = str(pidPriors)[1:-1]  # remove brackets
    trackQuality = 'thetaInCDCAcceptance and nCDCHits>20'
    if custom_cuts != '':
        trackQuality = custom_cuts
    for name in _chargednames:
        fillParticleList(f'{name}+:{_mostLikelyList}{suffix}',
                         f'pidIsMostLikely({args}) > 0 and {trackQuality}', True, path=path)
