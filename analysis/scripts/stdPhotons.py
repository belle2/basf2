#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# Author(s): Torben Ferber (ferber@physics.ubc.ca)
#            Savino Longo (savino.longo@desy.de)
#
########################################################

from modularAnalysis import fillParticleList, cutAndCopyList


def stdPhotons(listtype='loose', path=None):
    """
    Function to prepare one of several standardized types of photon lists:

    - 'gamma:all' with no cuts this will be polluted by tracks from outside the acceptance
    - 'gamma:cdc' all clusters inside the CDC tracking acceptance
    - 'gamma:loose' (default) with some loose quality selections
    - 'gamma:tight' like loose but with higher energy cuts depending on detector regions
    - 'gamma:pi0eff60_Jan2020' gamma list for 60% pi0 efficiency list, optimized in January 2020
    - 'gamma:pi0eff50_Jan2020' gamma list for 50% pi0 efficiency list, optimized in January 2020
    - 'gamma:pi0eff40_Jan2020' gamma list for 40% pi0 efficiency list, optimized in January 2020
    - 'gamma:pi0eff30_Jan2020' gamma list for 30% pi0 efficiency list, optimized in January 2020
    - 'gamma:pi0eff20_Jan2020' gamma list for 20% pi0 efficiency list, optimized in January 2020
    - 'gamma:pi0eff10_Jan2020' gamma list for 10% pi0 efficiency list, optimized in January 2020
    - 'gamma:pi0' gamma list for pi0 list
    - 'gamma:pi0highE' gamma list for pi0 list, high energy selection

    -  For latest pi0 recommendations see https://confluence.desy.de/display/BI/Neutrals+Performance

    @param listtype name of standard list
    @param path     modules are added to this path
    """

    # all photons (all neutral ECLClusters that have the c_nPhotons hypothesis)
    if listtype == 'all':
        fillParticleList('gamma:all', '', True, path)
    # all photons within the cdc tracking acceptance: remove un track-matched
    # electrons from outside the tracking acceptance
    elif listtype == 'cdc':
        fillParticleList(
            'gamma:cdc',
            'theta > 0.296706 and theta < 2.61799',
            True,
            path)
    # clusterErrorTiming < 1e6 removes failed waveform fits, this is not an actual timing cut. A 99% efficiency cut
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
    elif listtype == 'pi0eff10_Jan2020':
        fillParticleList(
            'gamma:pi0eff10_Jan2020',
            '[clusterReg==1 and E>0.200] or [clusterReg==2 and E>0.100] or [clusterReg==3 and E>0.180 and clusterE1E9>0.5]',
            True,
            path)
    elif listtype == 'pi0eff20_Jan2020':
        fillParticleList(
            'gamma:pi0eff20_Jan2020',
            '[clusterReg==1 and E>0.120] or [clusterReg==2 and E>0.030] or [clusterReg==3 and E>0.080 and clusterE1E9>0.4]',
            True,
            path)
    elif listtype == 'pi0eff30_Jan2020' or listtype == 'pi0eff40_Jan2020':
        fillParticleList(
                f'gamma:{listtype}',
                '[clusterReg==1 and E>0.080] or [clusterReg==2 and E>0.030] or [clusterReg==3 and E>0.060 ]',
                True,
                path)
    elif listtype == 'pi0eff50_Jan2020':
        fillParticleList(
            'gamma:pi0eff50_Jan2020',
            '[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.025] or [clusterReg==3 and E>0.040]',
            True,
            path)
    elif listtype == 'pi0eff60_Jan2020':
        fillParticleList(
            'gamma:pi0eff60_Jan2020',
            '[clusterReg==1 and E>0.0225] or [clusterReg==2 and E>0.020] or [clusterReg==3 and E>0.020]',
            True,
            path)
    else:
        raise ValueError(f"\"{listtype}\" is none of the allowed standardized types of photon lists!")

# Used in skimming code


def loadStdSkimPhoton(path):
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
def loadStdGoodBellePhoton(path):
    """
    Load the Belle goodBelle list. Creates a ParticleList named
    'gamma:goodBelle' with '0.5 < :b2:var:`goodBelleGamma` < 1.5'

    Warning:
        Should only be used for Belle analyses using `b2bii`.

    Parameters:
        path (basf2.Path): the path to load the modules
    """
    fillParticleList('gamma:goodBelle', '0.5 < goodBelleGamma < 1.5', True, path)
