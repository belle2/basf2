#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Author(s): Torben Ferber (ferber@physics.ubc.ca)
#
# more documentation: https://confluence.desy.de/x/I3I0Aw
#
########################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *


def stdPi0s(listtype='veryLoose', path=analysis_main):
    """
    Function to prepare one of several standardized types of pi0 lists:

    - 'pi0:all' using gamma:all
    - 'pi0:veryLoose' (default) gamma:pi0, mass range selection
    - 'pi0:loose' gamma:pi0highE, mass range selection
    - 'pi0:eff20' gamma:pi0eff20, mass range selection
    - 'pi0:eff30' gamma:pi0eff30, mass range selection
    - 'pi0:eff40' gamma:pi0eff40, mass range selection
    - 'pi0:eff50' gamma:pi0eff50, mass range selection
    - 'pi0:eff60' gamma:pi0eff60, mass range selection

    @param listtype name of standard list
    @param path     modules are added to this path
    """
    if listtype == 'all':
        fillParticleList('gamma:all', '', True, path)
        reconstructDecay('pi0:all -> gamma:all gamma:all', '', 1, True, path)
        matchMCTruth('pi0:all', path)
    elif listtype == 'veryLoose':
        fillParticleList('gamma:pi0', '', True, path)
        reconstructDecay('pi0:veryLoose -> gamma:pi0 gamma:pi0', '0.09 < M < 0.165', 1, True, path)
        matchMCTruth('pi0:veryLoose', path)
    elif listtype == 'loose':
        fillParticleList('gamma:pi0highE', '', True, path)
        reconstructDecay('pi0:loose -> gamma:pi0highE gamma:pi0highE', '0.1 < M < 0.160', 1, True, path)
        matchMCTruth('pi0:loose', path)

    elif listtype == 'eff20':
        fillParticleList('gamma:pi0eff20', '', True, path)
        reconstructDecay('pi0:eff20 -> gamma:pi0eff20 gamma:pi0eff20', '0.129 < M < 0.137', 1, True, path)
        matchMCTruth('pi0:eff20', path)
    elif listtype == 'eff30':
        fillParticleList('gamma:pi0eff30', '', True, path)
        reconstructDecay('pi0:eff30 -> gamma:pi0eff30 gamma:pi0eff30', '0.126 < M < 0.139', 1, True, path)
        matchMCTruth('pi0:eff30', path)
    elif listtype == 'eff40':
        fillParticleList('gamma:pi0eff40', '', True, path)
        reconstructDecay('pi0:eff40 -> gamma:pi0eff40 gamma:pi0eff40', '0.124 < M < 0.140', 1, True, path)
        matchMCTruth('pi0:eff40', path)
    elif listtype == 'eff50':
        fillParticleList('gamma:pi0eff50', '', True, path)
        reconstructDecay('pi0:eff50 -> gamma:pi0eff50 gamma:pi0eff50', '0.09 < M < 0.175', 1, True, path)
        matchMCTruth('pi0:eff50', path)
    elif listtype == 'eff60':
        fillParticleList('gamma:pi0eff60', '', True, path)
        reconstructDecay('pi0:eff60 -> gamma:pi0eff60 gamma:pi0eff60', '0.09 < M < 0.175', 1, True, path)
        matchMCTruth('pi0:eff60', path)

    elif listtype == 'skim':  # no MC truth matching, vertex fit
        fillParticleList('gamma:pi0', '', True, path)
        reconstructDecay('pi0:skim -> gamma:pi0 gamma:pi0', '0.09 < M < 0.165', 1, True, path)
        massKFit('pi0:skim', 0.0, '', path)

    # lists with mass constraints fits
    elif listtype == 'veryLooseFit':
        stdPi0s(listtype='veryLoose')
        cutAndCopyList('pi0:veryLooseFit', 'pi0:veryLoose', '', True, path)
        massKFit('pi0:veryLooseFit', 0.0, '', path)
    elif listtype == 'looseFit':
        stdPi0s(listtype='loose')
        cutAndCopyList('pi0:looseFit', 'pi0:loose', '', True, path)
        massKFit('pi0:looseFit', 0.0, '', path)
    elif listtype == 'allFit':
        stdPi0s(listtype='all')
        cutAndCopyList('pi0:allFit', 'pi0:all', '', True, path)
        massKFit('pi0:allFit', 0.0, '', path)
    elif listtype == 'eff20Fit':
        stdPi0s(listtype='eff20')
        cutAndCopyList('pi0:eff20Fit', 'pi0:eff20', '', True, path)
        massKFit('pi0:eff20Fit', 0.0, '', path)
    elif listtype == 'eff30Fit':
        stdPi0s(listtype='eff30')
        cutAndCopyList('pi0:eff30Fit', 'pi0:eff30', '', True, path)
        massKFit('pi0:eff30Fit', 0.0, '', path)
    elif listtype == 'eff40Fit':
        stdPi0s(listtype='eff40')
        cutAndCopyList('pi0:eff40Fit', 'pi0:eff40', '', True, path)
        massKFit('pi0:eff40Fit', 0.0, '', path)
    elif listtype == 'eff50Fit':
        stdPi0s(listtype='eff50')
        cutAndCopyList('pi0:eff50Fit', 'pi0:eff50', '', True, path)
        massKFit('pi0:eff50Fit', 0.0, '', path)
    elif listtype == 'eff60Fit':
        stdPi0s(listtype='eff60')
        cutAndCopyList('pi0:eff60Fit', 'pi0:eff60', '', True, path)
        massKFit('pi0:eff60Fit', 0.0, '', path)

# pi0 list for skims (and ONLY for skims)


def loadStdSkimPi0(path=analysis_main):
    stdPi0s('skim', path)

# deprecated
# ------------------------------------------------------------------------------


def loadStdPi0(listtype='veryLoose', path=analysis_main):
    stdPi0s(listtype, path)

# not recommended! only here for backwards compatibility


def loadStdLoosePi0(path=analysis_main):
    loadStdPi0(path)
    cutAndCopyList('pi0:loose', 'pi0:all', '-0.6 < extraInfo(BDT) < 1.0',
                   True, path)


def loadStdGoodPi0(path=analysis_main):
    loadStdPi0(path)
    cutAndCopyList('pi0:good', 'pi0:all', '0.5 < extraInfo(BDT) < 1.0', True,
                   path)
