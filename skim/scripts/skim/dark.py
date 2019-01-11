#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the dark sector physics working group """

__authors__ = [
    "Sam Cunliffe",
    "Michael De Nuccio",
    "Ilya Komarov"
]


import pdg
from modularAnalysis import cutAndCopyList, applyEventCuts, fillParticleList, \
    reconstructDecay


def SinglePhotonDarkList(path):
    """
    Note:
        * Single photon skim list for the dark photon analysis
        * Skim code: 18020100
        * Physics channel: ee → A'γ; A' → invisible
        * Skim category: physics, dark sector

    Build the list  of single photon candidates for the dark photon to
    invisible final state analysis.

    Parameters:
        path (basf2.Path) the path to add the skim

    Returns:
        list name of the skim candidates
    """
    __author__ = "Sam Cunliffe"

    # no good tracks in the event
    cleaned = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15'  # cm, cm, GeV/c
    applyEventCuts('nCleanedTracks(' + cleaned + ') < 1', path=path)

    # no other photon above 100 MeV
    angle = '0.296706 < theta < 2.61799'  # rad, (17 -- 150 deg)
    minimum = 'E > 0.1'  # GeV
    cutAndCopyList('gamma:100', 'gamma:all', minimum + ' and ' + angle, path=path)
    applyEventCuts('0 < nParticlesInList(gamma:100) < 2', path=path)

    # all remaining single photon events (== candidates) with region
    # dependent minimum energy in GeV
    region_dependent = ' [clusterReg ==  2 and useCMSFrame(E) > 1.0] or '  # barrel
    region_dependent += '[clusterReg ==  1 and useCMSFrame(E) > 2.0] or '  # fwd
    region_dependent += '[clusterReg ==  3 and useCMSFrame(E) > 2.0] or '  # bwd
    region_dependent += '[clusterReg == 11 and useCMSFrame(E) > 2.0] or '  # between fwd and barrel
    region_dependent += '[clusterReg == 13 and useCMSFrame(E) > 2.0] '     # between bwd and barrel
    cutAndCopyList('gamma:singlePhoton', 'gamma:100', region_dependent, path=path)
    return ['gamma:singlePhoton']


def _addALPToPDG():
    """ Adds the ALP codes to the basf2 pdg instance """
    __author__ = "Michael De Nuccio"
    pdg.add_particle('beam', 55, 999., 999., 0, 0)
    pdg.add_particle('ALP', 9000006, 999., 999., 0, 0)


def _initialALP(path):
    """
    An list builder function for the ALP decays. Part of the
    `ALP3GammaList` skim functions.

    Parameters:
        path (basf2.Path) the path to add the skim

    Returns:
        list name of the ALP decays candidates
    """
    __author__ = "Michael De Nuccio"

    # no cuts applied on ALP
    ALPcuts = ''

    # applying a lab frame energy cut to the daughter photons
    fillParticleList(
        'gamma:cdcAndMinimumEnergy',
        'E >= 0.1 and theta >= 0.297 and theta <= 2.618',
        True, path=path
    )

    # defining the decay string
    ALPchannels = ['gamma:cdcAndMinimumEnergy  gamma:cdcAndMinimumEnergy']
    ALPList = []

    # creating an ALP from the daughter photons
    for chID, channel in enumerate(ALPchannels):
        mode = 'ALP:' + str(chID) + ' -> ' + channel
        print(mode)
        reconstructDecay(mode, ALPcuts, chID, path=path)

        ALPList.append('ALP:' + str(chID))

    Lists = ALPList
    return Lists


def ALP3GammaList(path):
    """
    Note:
        * Neutral dark sector skim list for the ALP 3-photon analysis,
        * Skim code:   18020300
        * Physics channel: ee → aγ; a → γγ
        * Skim category: physics, dark sector

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """
    __author__ = "Michael De Nuccio"

    _addALPToPDG()

    # applying invariant mass cut on the beam particle
    beamcuts = 'InvM >= formula(0.8 * Ecms) and InvM <= formula(1.05 * Ecms) and maxWeightedDistanceFromAverageECLTime <= 2'

    ALPList = _initialALP(path=path)

    # applying a lab frame energy cut to the recoil photon
    fillParticleList('gamma:minimumEnergy', 'E >= 0.1', True, path=path)
    beamList = []

    # reconstructing decay using the reconstructed ALP
    # from previous function and adding the recoil photon
    for chID, channel in enumerate(ALPList):
        mode = 'beam:' + str(chID) + ' -> gamma:minimumEnergy ' + channel
        print(mode)
        reconstructDecay(mode, beamcuts, chID, path=path)
        beamList.append('beam:' + str(chID))
    return beamList


def LFVZpInvisibleList(path):
    """
    Note:
        * Lepton flavour violating Z' skim, Z' to invisible FS
        * Skim code:  18520400
        * Physics channel: ee --> e mu Z'; Z' --> invisible
        * Skim category: physics, dark sector

    The skim list for the LFV Z' to invisible final state search

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list containing the candidate names
    """
    __author__ = "Ilya Komarov"

    lfvzp_list = []

    # Some loose PID cuts for leptons
    muID_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pidProbabilityExpert(13, all) > 0.1'
    eID_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pidProbabilityExpert(11, all) > 0.1'

    # We want exaclty 2 good tracks
    Event_cuts = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'

    cutAndCopyList('mu+:lfvzp', 'mu+:all', muID_cuts, path=path)
    cutAndCopyList('e+:lfvzp', 'e+:all', eID_cuts, path=path)

    # Z' to invisible
    LFVZpInvChannel = 'mu+:lfvzp e-:lfvzp'
    reconstructDecay('vpho:invlfvzp -> ' + LFVZpInvChannel, Event_cuts, path=path)
    lfvzp_list.append('vpho:invlfvzp')

    return lfvzp_list


def LFVZpVisibleList(path):
    """
    Note:
        * Lepton flavour violating Z' skim, Z' to visible FS
        * Skim code:  18520500
        * Physics channel: ee --> e mu Z'; Z' --> e mu
        * Skim category: physics, dark sector

    The skim list for the LFV Z' to visible final state search

    Parameters:
        path (basf2.Path) the path to add the skim list builders

    Returns:
        list containing the candidate names
    """
    __author__ = "Ilya Komarov"

    lfvzp_list = []

    # Here we just want four gpood tracks to be reconstructed
    track_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 4'

    cutAndCopyList('e+:lfvzp', 'e+:all', track_cuts, path=path)

    # Z' to lfv: fully reconstructed
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp e-:lfvzp e-:lfvzp'

    reconstructDecay('vpho:vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis, path=path)

    lfvzp_list.append('vpho:vislfvzp')

    # Z' to lfv: part reco
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp e-:lfvzp'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 3'

    reconstructDecay('vpho:3tr_vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis, path=path)

    lfvzp_list.append('vpho:3tr_vislfvzp')

    # Z' to lfv: two same-sign tracks
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    reconstructDecay('vpho:2tr_vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis, path=path)

    lfvzp_list.append('vpho:2tr_vislfvzp')

    return lfvzp_list
