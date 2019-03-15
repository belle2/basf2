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


def stdPhotons(listtype='loose', path=analysis_main):
    """
    Function to prepare one of several standardized types of photon lists:

    - 'gamma:all' with no cuts this will be polluted by tracks from outside the acceptance
    - 'gamma:cdc' all clusters inside the CDC tracking acceptance
    - 'gamma:loose' (default) with some loose quality selections
    - 'gamma:tight' like loose but with higher energy cuts depending on detector regions
    - 'gamma:pi0eff60' gamma list for 60% pi0 efficiency list
    - 'gamma:pi0eff50' gamma list for 50% pi0 efficiency list
    - 'gamma:pi0eff40' gamma list for 40% pi0 efficiency list
    - 'gamma:pi0eff30' gamma list for 30% pi0 efficiency list
    - 'gamma:pi0eff20' gamma list for 20% pi0 efficiency list
    - 'gamma:pi0' gamma list for pi0 list
    - 'gamma:pi0highE' gamma list for pi0 list, high energy selection

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    # all photons (reconstructed using the N1 clustering)
    if listtype == 'all':
        fillParticleList('gamma:all', 'clusterHasNPhotons', True, path)
    # all photons within the cdc tracking acceptance: remove un track-matched
    # electrons from outside the tracking acceptance
    elif listtype == 'cdc':
        stdPhotons('all', path)
        cutAndCopyList(
            'gamma:cdc',
            'gamma:all',
            'theta > 0.296706 and theta < 2.61799',
            True,
            path)
    # clusterErrorTiming < 1e6 removes failed waveform fits, this is not an actual timing cut. A 99% efficienct cut
    # is already applied on mdst level for photons with E < 50 MeV.
    elif listtype == 'loose':
        stdPhotons('cdc', path)
        cutAndCopyList(
            'gamma:loose',
            'gamma:cdc',
            'clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]',
            True,
            path)
    # additional region dependent energy cuts
    elif listtype == 'tight':
        stdPhotons('loose', path)
        cutAndCopyList(
            'gamma:tight',
            'gamma:loose',
            '[clusterReg == 1 and E > 0.05] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.075]',
            True,
            path)
    elif listtype == 'pi0eff20':
        stdPhotons('all', path)
        cutAndCopyList(
            'gamma:pi0eff20',
            'gamma:all', 'theta > 0.296706 and theta < 2.61799 and \
            [[clusterReg == 1 and E > 0.075] or [clusterReg == 2 and E > 0.075] or [clusterReg == 3 and E > 0.075]] and \
            [abs(clusterTiming) < formula(0.1 * clusterErrorTiming) or E > 0.1] and [clusterE1E9 > 0.7 or E > 0.1] ',
            True,
            path)
    elif listtype == 'pi0eff30':
        stdPhotons('all', path)
        cutAndCopyList(
            'gamma:pi0eff30',
            'gamma:all', 'theta > 0.296706 and theta < 2.61799 and \
            [[clusterReg == 1 and E > 0.075] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.075]] and \
            [abs(clusterTiming) < formula(0.5 * clusterErrorTiming) or E > 0.1] and [clusterE1E9 > 0.7 or E > 0.1] ',
            True,
            path)
    elif listtype == 'pi0eff40':
        stdPhotons('all', path)
        cutAndCopyList(
            'gamma:pi0eff40',
            'gamma:all', 'theta > 0.296706 and theta < 2.61799 and \
            [[clusterReg == 1 and E > 0.075] or [clusterReg == 2 and E > 0.03] or [clusterReg == 3 and E > 0.075]] and \
            [abs(clusterTiming) < formula(0.5 * clusterErrorTiming) or E > 0.1] and [clusterE1E9 > 0.5 or E > 0.1] ',
            True,
            path)
    elif listtype == 'pi0eff50':
        stdPhotons('all', path)
        cutAndCopyList(
            'gamma:pi0eff50',
            'gamma:all', 'theta > 0.296706 and theta < 2.61799 and \
            [[clusterReg == 1 and E > 0.05] or [clusterReg == 2 and E > 0.03] or [clusterReg == 3 and E > 0.05]] and \
            [abs(clusterTiming) < formula(1.0 * clusterErrorTiming) or E > 0.1] and [clusterE1E9 > 0.3 or E > 0.1] ',
            True,
            path)
    elif listtype == 'pi0eff60':
        stdPhotons('all', path)
        cutAndCopyList(
            'gamma:pi0eff60',
            'gamma:all', 'theta > 0.296706 and theta < 2.61799 and \
            [[clusterReg == 1 and E > 0.03] or [clusterReg == 2 and E > 0.02] or [clusterReg == 3 and E > 0.03]] and \
            [abs(clusterTiming) < formula(1.0 * clusterErrorTiming) or E > 0.1] and [clusterE1E9 > 0.3 or E > 0.1] ',
            True,
            path)
    elif listtype == 'pi0':
        stdPhotons('pi0eff60', path)
        cutAndCopyList('gamma:pi0', 'gamma:pi0eff60', '', True, path)
    elif listtype == 'pi0highE':
        stdPhotons('pi0eff60', path)
        cutAndCopyList('gamma:pi0highE', 'gamma:pi0eff60', 'E > 0.2', True, path)


# Used in skimming code
def loadStdSkimPhoton(path=analysis_main):
    """
    Function to prepare the skim photon lists.

    Warning:
        Should only be used by skims.

    Parameters:
        path (basf2.Path): modules are added to this path

    """
    stdPhotons('loose', path)
    cutAndCopyList(
        'gamma:skim',
        'gamma:loose',
        '',
        True,
        path)

# Only used for Belle via b2bii


def loadStdGoodBellePhoton(path=analysis_main):
    """
    Load the Belle goodBelle list. Creates a ParticleList named
    'gamma:goodBelle' with '0.5 < :b2:var:`goodBelleGamma` < 1.5'

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    stdPhotons('all', path)
    cutAndCopyList('gamma:goodBelle', 'gamma:all', '0.5 < goodBelleGamma < 1.5', True, path)
