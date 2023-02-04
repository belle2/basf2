#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma


def stdPhotons(
        listtype='loose',
        path=None,
        beamBackgroundMVAWeight="",
        fakePhotonMVAWeight="",
        biasCorrectionTable=""):
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
        beamBackgroundMVAWeight (str): type of weight file for beam background MVA; if empty, beam background MVA will not be used

                          .. tip::
                              Please refer to the
                              `Neutrals Performance Confluence page <https://confluence.desy.de/display/BI/Neutrals+Performance>`_
                              for information on the beam background MVA.

        fakePhotonMVAWeight (str): type of weight file for fake photon MVA; if empty, fake photon MVA will not be used

                          .. tip::
                              Please refer to the
                              `Neutrals Performance Confluence page <https://confluence.desy.de/display/BI/Neutrals+Performance>`_
                              for information on the fake photon MVA.

        biasCorrectionTable (str): correction table for the photon energy bias correction (should only be applied to data)

                          .. tip::
                              Please refer to the
                              `Neutrals Performance Confluence page <https://confluence.desy.de/display/BI/Neutrals+Performance>`_
                              for information on the names of available correction tables..
    """

    # all photons (all neutral ECLClusters that have the c_nPhotons hypothesis)
    if listtype == 'all':
        ma.fillParticleList('gamma:all', '', writeOut=True, path=path)
    # all photons within the cdc tracking acceptance: remove un track-matched
    # electrons from outside the tracking acceptance
    elif listtype == 'cdc':
        ma.fillParticleList(
            'gamma:cdc',
            'inCDCAcceptance',
            writeOut=True,
            path=path
        )
    # clusterErrorTiming < 1e6 removes failed waveform fits, this is not an actual timing cut. A 99% efficiency cut
    # is already applied on mdst level for photons with E < 50 MeV.
    elif listtype == 'loose':
        stdPhotons('cdc', path,
                   beamBackgroundMVAWeight=beamBackgroundMVAWeight,
                   fakePhotonMVAWeight=fakePhotonMVAWeight,
                   biasCorrectionTable=biasCorrectionTable)
        ma.cutAndCopyList(
            'gamma:loose',
            'gamma:cdc',
            'clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]',
            True,
            path)
    # additional region dependent energy cuts
    elif listtype == 'tight':
        stdPhotons('loose', path,
                   beamBackgroundMVAWeight=beamBackgroundMVAWeight,
                   fakePhotonMVAWeight=fakePhotonMVAWeight,
                   biasCorrectionTable=biasCorrectionTable)
        ma.cutAndCopyList(
            'gamma:tight',
            'gamma:loose',
            '[clusterReg == 1 and E > 0.05] or [clusterReg == 2 and E > 0.05] or [clusterReg == 3 and E > 0.075]',
            True,
            path)
    elif listtype == 'pi0eff10_May2020':
        ma.fillParticleList(
            'gamma:pi0eff10_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.200] or [clusterReg==2 and E>0.100] or [clusterReg==3 and E>0.180]] and [clusterE1E9>0.5]',
            writeOut=True,
            path=path
        )
    elif listtype == 'pi0eff20_May2020':
        ma.fillParticleList(
            'gamma:pi0eff20_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.120] or [clusterReg==2 and E>0.030] or [clusterReg==3 and E>0.080]] and [clusterE1E9>0.4]',
            writeOut=True,
            path=path
        )
    elif listtype == 'pi0eff30_May2020' or listtype == 'pi0eff40_May2020':
        ma.fillParticleList(
            f'gamma:{listtype}',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.080] or [clusterReg==2 and E>0.030] or [clusterReg==3 and E>0.060 ]]',
            writeOut=True,
            path=path
        )
    elif listtype == 'pi0eff50_May2020':
        ma.fillParticleList(
            'gamma:pi0eff50_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
            [[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.025] or [clusterReg==3 and E>0.040]]',
            writeOut=True,
            path=path
        )
    elif listtype == 'pi0eff60_May2020':
        ma.fillParticleList(
            'gamma:pi0eff60_May2020',
            '[clusterNHits>1.5] and [0.2967< clusterTheta<2.6180] and \
             [[clusterReg==1 and E>0.0225] or [clusterReg==2 and E>0.020] or [clusterReg==3 and E>0.020]]',
            writeOut=True,
            path=path
        )
    else:
        raise ValueError(f"\"{listtype}\" is none of the allowed standardized types of photon lists!")

    if listtype not in ['loose', 'tight']:
        if beamBackgroundMVAWeight:
            ma.getBeamBackgroundProbability(decayString=f'gamma:{listtype}', weight=beamBackgroundMVAWeight, path=path)
        if fakePhotonMVAWeight:
            ma.getFakePhotonProbability(decayString=f'gamma:{listtype}', weight=fakePhotonMVAWeight, path=path)
        if biasCorrectionTable:
            ma.correctEnergyBias(inputListNames=[f'gamma:{listtype}'], tableName=biasCorrectionTable, path=path)

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
    ma.cutAndCopyList(
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
    ma.fillParticleList('gamma:goodBelle', '0.5 < goodBelleGamma < 1.5', True, path)
