#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from modularAnalysis import fillParticleList, cutAndCopyList


def stdPhotons(listtype='loose', path=None, loadPhotonBeamBackgroundMVA=True):
    """
    Function to prepare one of several standardized types of photon lists:

    - 'gamma:all' with no cuts this will be polluted by tracks from outside the acceptance
    - 'gamma:cdc' all clusters inside the CDC tracking acceptance
    - 'gamma:loose' (default) with some loose quality selections
    - 'gamma:tight' like loose but with higher energy cuts depending on detector regions
    - 'gamma:pi0eff60_May2020' gamma list for 60% pi0 efficiency list, optimized in May 2020
    - 'gamma:pi0eff50_May2020' gamma list for 50% pi0 efficiency list, optimized in May 2020
    - 'gamma:pi0eff40_May2020' gamma list for 40% pi0 efficiency list, optimized in May 2020
    - 'gamma:pi0eff30_May2020' gamma list for 30% pi0 efficiency list, optimized in May 2020
    - 'gamma:pi0eff20_May2020' gamma list for 20% pi0 efficiency list, optimized in May 2020
    - 'gamma:pi0eff10_May2020' gamma list for 10% pi0 efficiency list, optimized in May 2020
    - 'gamma:pi0' gamma list for pi0 list
    - 'gamma:pi0highE' gamma list for pi0 list, high energy selection

    -  For latest pi0 recommendations see https://confluence.desy.de/display/BI/Neutrals+Performance

    Parameters:
        listtype (str): name of standard list
        path (basf2.Path):           modules are added to this path
        loadPhotonBeamBackgroundMVA (bool): If true, photon candidates will be assigned a beam background probability.
    """

    # all photons (all neutral ECLClusters that have the c_nPhotons hypothesis)
    if listtype == 'all':
        fillParticleList('gamma:all', '', writeOut=True, path=path,
                         loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA)
    # all photons within the cdc tracking acceptance: remove un track-matched
    # electrons from outside the tracking acceptance
    elif listtype == 'cdc':
        fillParticleList(
            'gamma:cdc',
            'inCDCAcceptance',
            writeOut=True,
            path=path,
            loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA
        )
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
    elif listtype == 'pi0eff10_May2020':
        fillParticleList(
            'gamma:pi0eff10_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.200] or [clusterReg==2 and E>0.100] or [clusterReg==3 and E>0.180]] and [clusterE1E9>0.5]',
            writeOut=True,
            path=path,
            loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA
        )
    elif listtype == 'pi0eff20_May2020':
        fillParticleList(
            'gamma:pi0eff20_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.120] or [clusterReg==2 and E>0.030] or [clusterReg==3 and E>0.080]] and [clusterE1E9>0.4]',
            writeOut=True,
            path=path,
            loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA
        )
    elif listtype == 'pi0eff30_May2020' or listtype == 'pi0eff40_May2020':
        fillParticleList(
            f'gamma:{listtype}',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.080] or [clusterReg==2 and E>0.030] or [clusterReg==3 and E>0.060 ]]',
            writeOut=True,
            path=path,
            loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA
        )
    elif listtype == 'pi0eff50_May2020':
        fillParticleList(
            'gamma:pi0eff50_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
            [[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.025] or [clusterReg==3 and E>0.040]]',
            writeOut=True,
            path=path,
            loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA
        )
    elif listtype == 'pi0eff60_May2020':
        fillParticleList(
            'gamma:pi0eff60_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.0225] or [clusterReg==2 and E>0.020] or [clusterReg==3 and E>0.020]]',
            writeOut=True,
            path=path,
            loadPhotonBeamBackgroundMVA=loadPhotonBeamBackgroundMVA
        )
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
