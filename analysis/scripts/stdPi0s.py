#!/usr/bin/env python3

########################################################
#
# Author(s): Torben Ferber (ferber@physics.ubc.ca)
#            Savino Longo(savino.longo@desy.de)
#
# more documentation: https://confluence.desy.de/x/I3I0Aw
#
########################################################

import modularAnalysis as ma
from stdPhotons import stdPhotons
from vertex import kFit
from basf2 import B2WARNING


def stdPi0s(listtype="eff60_May2020", path=None):
    """
    Function to prepare one of several standardized types of pi0 lists:

    - 'all' using gamma:all
    - 'eff10_May2020' gamma:pi0eff10_May2020, mass range selection, 10% pi0 efficiency list, optimized in May 2020
    - 'eff20_May2020' gamma:pi0eff20_May2020, mass range selection, 20% pi0 efficiency list, optimized in May 2020
    - 'eff30_May2020' gamma:pi0eff30_May2020, mass range selection, 30% pi0 efficiency list, optimized in May 2020
    - 'eff40_May2020' gamma:pi0eff40_May2020, mass range selection, 40% pi0 efficiency list, optimized in May 2020
    - 'eff50_May2020' gamma:pi0eff50_May2020, mass range selection, 50% pi0 efficiency list, optimized in May 2020
    - 'eff60_May2020' gamma:pi0eff60_May2020, mass range selection, 60% pi0 efficiency list, optimized in May 2020

    You can also append "Fit" to the listtype which will run a mass fit and
    require that the fit did not fail. For example: "pi0:eff50_May2020Fit" is the 50%
    efficiency list plus a not-failing mass fit.

    Parameters:
        listtype (str): name of standard list
        path (basf2.Path): modules are added to this path
    """

    B2WARNING("stdPi0s is loading \"May2020\" pi0 recommendations. Please check Neutrals Performance Confluence"
              " page for most up-to-date pi0 recommendations.")

    if listtype == 'all':
        stdPhotons('all', path)
        ma.reconstructDecay('pi0:all -> gamma:all gamma:all', '', 1, True, path)
        ma.matchMCTruth('pi0:all', path)
    elif 'eff10_May2020' == listtype:
        stdPhotons('pi0eff10_May2020', path)
        ma.reconstructDecay('pi0:eff10_May2020 -> gamma:pi0eff10_May2020 gamma:pi0eff10_May2020',
                            '0.127<InvM<0.139 and -0.9<daughterDiffOf(0,1)<0.9 and daughterAngle(0,1)<0.8',
                            1,
                            True,
                            path)
        ma.matchMCTruth('pi0:eff10_May2020', path)
    elif 'eff20_May2020' == listtype:
        stdPhotons('pi0eff20_May2020', path)
        ma.reconstructDecay('pi0:eff20_May2020 -> gamma:pi0eff20_May2020 gamma:pi0eff20_May2020',
                            '0.121<InvM<0.142 and -1.0<daughterDiffOf(0,1)<1.0 and daughterAngle(0,1)<0.9',
                            1,
                            True,
                            path)
        ma.matchMCTruth('pi0:eff20_May2020', path)
    elif 'eff30_May2020' == listtype:
        stdPhotons('pi0eff30_May2020', path)
        ma.reconstructDecay('pi0:eff30_May2020 -> gamma:pi0eff30_May2020 gamma:pi0eff30_May2020',
                            '0.120<InvM<0.145 and -1.5<daughterDiffOf(0,1)<1.5 and daughterAngle(0,1)<1.4',
                            1,
                            True,
                            path)
        ma.matchMCTruth('pi0:eff30_May2020', path)
    elif 'eff40_May2020' == listtype:
        stdPhotons('pi0eff40_May2020', path)
        ma.reconstructDecay('pi0:eff40_May2020 -> gamma:pi0eff40_May2020 gamma:pi0eff40_May2020', '0.120<InvM<0.145', 1, True, path)
        ma.matchMCTruth('pi0:eff40_May2020', path)
    elif 'eff50_May2020_nomcmatch' == listtype:
        stdPhotons('pi0eff50_May2020', path)
        ma.reconstructDecay(
            'pi0:eff50_May2020_nomcmatch -> gamma:pi0eff50_May2020 gamma:pi0eff50_May2020',
            '0.105<InvM<0.150',
            1,
            True,
            path)
    elif 'eff50_May2020' == listtype:
        stdPi0s('eff50_May2020_nomcmatch', path)
        ma.cutAndCopyList('pi0:eff50_May2020', 'pi0:eff50_May2020_nomcmatch', '', True, path)
        ma.matchMCTruth('pi0:eff50_May2020', path)
    elif 'eff60_May2020' == listtype:
        stdPhotons('pi0eff60_May2020', path)
        ma.reconstructDecay('pi0:eff60_May2020 -> gamma:pi0eff60_May2020 gamma:pi0eff60_May2020', '0.03<InvM', 1, True, path)
        ma.matchMCTruth('pi0:eff60_May2020', path)

    # skim list(s)
    elif listtype == 'skim':
        stdPi0s('eff50_May2020_nomcmatch', path)
        ma.cutAndCopyList('pi0:skim', 'pi0:eff50_May2020_nomcmatch', '', True, path)
        kFit('pi0:skim', 0.0, 'mass', path=path)

    # same lists with, but with  mass constraints fits
    elif listtype == 'allFit':
        stdPi0s('all', path)
        ma.cutAndCopyList('pi0:allFit', 'pi0:all', '', True, path)
        kFit('pi0:allFit', 0.0, 'mass', path=path)
    elif listtype == 'eff10_May2020Fit':
        stdPi0s('eff10_May2020', path)
        ma.cutAndCopyList('pi0:eff10_May2020Fit', 'pi0:eff10_May2020', '', True, path)
        kFit('pi0:eff10_May2020Fit', 0.0, 'mass', path=path)
    elif listtype == 'eff20_May2020Fit':
        stdPi0s('eff20_May2020', path)
        ma.cutAndCopyList('pi0:eff20_May2020Fit', 'pi0:eff20_May2020', '', True, path)
        kFit('pi0:eff20_May2020Fit', 0.0, 'mass', path=path)
    elif listtype == 'eff30_May2020Fit':
        stdPi0s('eff30_May2020', path)
        ma.cutAndCopyList('pi0:eff30_May2020Fit', 'pi0:eff30_May2020', '', True, path)
        kFit('pi0:eff30_May2020Fit', 0.0, 'mass', path=path)
    elif listtype == 'eff40_May2020Fit':
        stdPi0s('eff40_May2020', path)
        ma.cutAndCopyList('pi0:eff40_May2020Fit', 'pi0:eff40_May2020', '', True, path)
        kFit('pi0:eff40_May2020Fit', 0.0, 'mass', path=path)
    elif listtype == 'eff50_May2020Fit':
        stdPi0s('eff50_May2020', path)
        ma.cutAndCopyList('pi0:eff50_May2020Fit', 'pi0:eff50_May2020', '', True, path)
        kFit('pi0:eff50_May2020Fit', 0.0, 'mass', path=path)
    elif listtype == 'eff60_May2020Fit':
        stdPi0s('eff60_May2020', path)
        ma.cutAndCopyList('pi0:eff60_May2020Fit', 'pi0:eff60_May2020', '', True, path)
        kFit('pi0:eff60_May2020Fit', 0.0, 'mass', path=path)
    else:
        raise ValueError(f"\"{listtype}\" is none of the allowed standardized types of pi0 lists!")

# pi0 list for skims (and ONLY for skims)


def loadStdSkimPi0(path):
    """
    Function to prepare the skim pi0 lists.

    Warning:
        Should only be used by skims.

    Parameters:
        path (basf2.Path) modules are added to this path

    """
    stdPi0s('skim', path)
