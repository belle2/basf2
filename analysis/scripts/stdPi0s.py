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
    - 'pi0:veryLoose' gamma:pi0, mass range selection
    - 'pi0:loose' gamma:pi0highE, mass range selection
    - 'pi0:eff20' gamma:pi0eff20, mass range selection, 20% pi0 efficiency list
    - 'pi0:eff30' gamma:pi0eff30, mass range selection, 30% pi0 efficiency list
    - 'pi0:eff40' gamma:pi0eff40, mass range selection, 40% pi0 efficiency list
    - 'pi0:eff50' gamma:pi0eff50, mass range selection, 50% pi0 efficiency list
    - 'pi0:eff60' gamma:pi0eff60, mass range selection, 60% pi0 efficiency list (default)

    @param listtype name of standard list
    @param path     modules are added to this path
    """
    if listtype == 'all':
        stdPhotons('all')
        reconstructDecay('pi0:all -> gamma:all gamma:all', '', 1, True, path)
        matchMCTruth('pi0:all', path)
    elif listtype == 'eff20':
        stdPhotons('pi0eff20')
        reconstructDecay('pi0:eff20 -> gamma:pi0eff20 gamma:pi0eff20', '0.129 < M < 0.137', 1, True, path)
        matchMCTruth('pi0:eff20', path)
    elif listtype == 'eff30':
        stdPhotons('pi0eff30')
        reconstructDecay('pi0:eff30 -> gamma:pi0eff30 gamma:pi0eff30', '0.126 < M < 0.139', 1, True, path)
        matchMCTruth('pi0:eff30', path)
    elif listtype == 'eff40':
        stdPhotons('pi0eff40')
        reconstructDecay('pi0:eff40 -> gamma:pi0eff40 gamma:pi0eff40', '0.124 < M < 0.140', 1, True, path)
        matchMCTruth('pi0:eff40', path)
    elif listtype == 'eff50':
        stdPhotons('pi0eff50')
        reconstructDecay('pi0:eff50 -> gamma:pi0eff50 gamma:pi0eff50', '0.09 < M < 0.175', 1, True, path)
        matchMCTruth('pi0:eff50', path)
    elif listtype == 'eff60_nomcmatch':
        stdPhotons('pi0eff60')
        reconstructDecay('pi0:eff60_nomcmatch -> gamma:pi0eff60 gamma:pi0eff60', '0.075 < M < 0.175', 1, True, path)
    elif listtype == 'eff60':
        stdPi0s(listtype='eff60_nomcmatch')
        cutAndCopyList('pi0:eff60', 'pi0:eff60_nomcmatch', '', True, path)
        matchMCTruth('pi0:eff60', path)
    elif listtype == 'veryLoose':
        stdPi0s(listtype='eff60')
        cutAndCopyList('pi0:veryLoose', 'pi0:eff60', '', True, path)
    elif listtype == 'loose':
        stdPi0s(listtype='eff40')
        cutAndCopyList('pi0:loose', 'pi0:eff40', '', True, path)

    # skim list(s)
    elif listtype == 'skim':
        stdPi0s(listtype='eff60_nomcmatch')
        cutAndCopyList('pi0:skim', 'pi0:eff60_nomcmatch', '', True, path)
        massKFit('pi0:skim', 0.0, '', path)

    # same lists with, but with  mass constraints fits
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
