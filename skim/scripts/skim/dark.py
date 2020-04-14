#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the dark sector physics working group """

__authors__ = [
    "Sam Cunliffe",
    "Michael De Nuccio",
    "Ilya Komarov",
    "Giacomo De Pietro",
    "Xing-Yu Zhou"
]


import basf2 as b2
import pdg
import modularAnalysis as ma
from skimExpertFunctions import ifEventPasses


def SinglePhotonDarkList(path):
    """
    Single photon skim list for the dark photon analysis

    **Skim code**: 18020100

    **Physics channel**: ee → A'γ; A' → invisible

    **Skim category**: physics, dark sector

    Build the list of single photon candidates for the dark photon to
    invisible final state analysis.

    Parameters:
        path (basf2.Path): the path to add the skim

    Returns:
        list name of the skim candidates
    """
    __author__ = "Sam Cunliffe"

    # no good tracks in the event
    cleaned = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15'  # cm, cm, GeV/c

    # no other photon above 100 MeV
    angle = '0.296706 < theta < 2.61799'  # rad, (17 -- 150 deg)
    minimum = 'E > 0.1'  # GeV
    ma.cutAndCopyList('gamma:100', 'gamma:all', minimum + ' and ' + angle, path=path)
    path2 = b2.Path()
    ifEventPasses('0 < nParticlesInList(gamma:100) <  2 and nCleanedTracks(' + cleaned + ') < 1', conditional_path=path2, path=path)

    # all remaining single photon events (== candidates) with region
    # dependent minimum energy in GeV
    region_dependent = ' [clusterReg ==  2 and useCMSFrame(E) > 1.0] or '  # barrel
    region_dependent += '[clusterReg ==  1 and useCMSFrame(E) > 2.0] or '  # fwd
    region_dependent += '[clusterReg ==  3 and useCMSFrame(E) > 2.0] or '  # bwd
    region_dependent += '[clusterReg == 11 and useCMSFrame(E) > 2.0] or '  # between fwd and barrel
    region_dependent += '[clusterReg == 13 and useCMSFrame(E) > 2.0] '     # between bwd and barrel
    ma.cutAndCopyList('gamma:singlePhoton', 'gamma:100', region_dependent, path=path2)
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
        path (basf2.Path): the path to add the skim

    Returns:
        list name of the ALP decays candidates
    """
    __author__ = "Michael De Nuccio"

    # no cuts applied on ALP
    ALPcuts = ''

    # applying a lab frame energy cut to the daughter photons
    ma.fillParticleList(
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
        ma.reconstructDecay(mode, ALPcuts, chID, path=path)

        ALPList.append('ALP:' + str(chID))

    Lists = ALPList
    return Lists


def ALP3GammaList(path):
    """
    Neutral dark sector skim list for the ALP 3-photon analysis.

    **Skim code**: 18020300

    **Physics channel**: ee → aγ; a → γγ

    **Skim category**: physics, dark sector

    Parameters:
        path (basf2.Path): the path to add the skim list builders

    Returns:
        list name of the skim candidates
    """
    __author__ = "Michael De Nuccio"

    _addALPToPDG()

    # applying invariant mass cut on the beam particle
    beamcuts = 'InvM >= formula(0.8 * Ecms) and InvM <= formula(1.05 * Ecms) and maxWeightedDistanceFromAverageECLTime <= 2'

    ALPList = _initialALP(path=path)

    # applying a lab frame energy cut to the recoil photon
    ma.fillParticleList('gamma:minimumEnergy', 'E >= 0.1', True, path=path)
    beamList = []

    # reconstructing decay using the reconstructed ALP
    # from previous function and adding the recoil photon
    for chID, channel in enumerate(ALPList):
        mode = 'beam:' + str(chID) + ' -> gamma:minimumEnergy ' + channel
        print(mode)
        ma.reconstructDecay(mode, beamcuts, chID, path=path)
        beamList.append('beam:' + str(chID))
    return beamList


def LFVZpVisibleList(path):
    """
    Lepton flavour violating Z' skim, Z' to visible FS

    **Skim code**: 18520400

    **Physics channel**: ee --> e mu Z'; Z' --> e mu

    **Skim category**: physics, dark sector

    The skim list for the LFV Z' to visible final state search

    Parameters:
        path (basf2.Path): the path to add the skim list builders

    Returns:
        list containing the candidate names
    """
    __author__ = "Ilya Komarov"

    lfvzp_list = []

    # Here we just want four gpood tracks to be reconstructed
    track_cuts = 'abs(dz) < 2.0 and abs(dr) < 0.5'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 4'

    ma.cutAndCopyList('e+:lfvzp', 'e+:all', track_cuts, path=path)

    # Z' to lfv: fully reconstructed
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp e-:lfvzp e-:lfvzp'

    ma.reconstructDecay('vpho:vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis, path=path)

    lfvzp_list.append('vpho:vislfvzp')

    # Z' to lfv: part reco
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp e-:lfvzp'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 3'

    ma.reconstructDecay('vpho:3tr_vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis, path=path, allowChargeViolation=True)

    lfvzp_list.append('vpho:3tr_vislfvzp')

    # Z' to lfv: two same-sign tracks
    LFVZpVisChannel = 'e+:lfvzp e+:lfvzp'
    Event_cuts_vis = 'nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2'
    ma.reconstructDecay('vpho:2tr_vislfvzp -> ' + LFVZpVisChannel, Event_cuts_vis, path=path, allowChargeViolation=True)

    lfvzp_list.append('vpho:2tr_vislfvzp')

    return lfvzp_list


def DimuonPlusMissingEnergyList(path):
    """
    Dimuon + missing energy skim,
    needed for :math:`e^{+}e^{-} \\to \mu^{+}\mu^{-} Z^{\prime}; \, Z^{\prime} \\to \mathrm{invisible}` and other searches

    **Skim code**: 18520100

    **Physics channel**: :math:`e^{+}e^{-} \\to \mu^{+}\mu^{-} \, +` missing energy

    **Skim category**: physics, dark sector

    Parameters:
        path (basf2.Path): the path to add the skim

    Returns:
        list containing the candidate names
    """
    __author__ = 'Giacomo De Pietro'

    dimuon_list = []
    skim_label = 'forDimuonMissingEnergySkim'
    dimuon_name = 'Z0:' + skim_label

    # Define some cuts
    fromIP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    muonID_cut = '[muonID > 0.3]'
    # We want exactly 2 tracks from IP
    dimuon_cut = '[nCleanedTracks(' + fromIP_cut + ') < 4]'
    # And the pair must have pt > 200 MeV in CMS frame
    dimuon_cut += ' and [useCMSFrame(pt) > 0.2]'

    # Reconstruct the dimuon candidate
    ma.cutAndCopyList('mu+:' + skim_label, 'mu+:all', fromIP_cut + ' and ' + muonID_cut, path=path)
    ma.reconstructDecay(dimuon_name + ' -> mu+:' + skim_label + ' mu-:' + skim_label, dimuon_cut, path=path)

    # And return the dimuon list
    dimuon_list.append(dimuon_name)
    return dimuon_list


def ElectronMuonPlusMissingEnergyList(path):
    """
    Electron-muon pair + missing energy skim,
    needed for :math:`e^{+}e^{-} \\to e^{\pm}\mu^{\mp} Z^{\prime}; \, Z^{\prime} \\to \mathrm{invisible}` and other searches

    **Skim code**: 18520200

    **Physics channel**: :math:`e^{+}e^{-} \\to e^{\pm}\mu^{\mp} \, +` missing energy

    **Skim category**: physics, dark sector

    Parameters:
        path (basf2.Path): the path to add the skim

    Returns:
        list containing the candidate names
    """
    __author__ = 'Giacomo De Pietro'

    emu_list = []
    skim_label = 'forElectronMuonMissingEnergySkim'
    emu_name = 'Z0:' + skim_label

    # Define some basic cuts
    fromIP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    electronID_cut = '[electronID > 0.3]'
    muonID_cut = '[muonID > 0.3]'
    # We require that the electron points to the barrel ECL + 10 degrees
    theta_cut = '[0.387 < theta < 2.421]'
    # We want exactly 2 tracks from IP
    emu_cut = '[nCleanedTracks(' + fromIP_cut + ') < 4]'
    # And the pair must have pt > 200 MeV in CMS frame
    emu_cut += ' and [useCMSFrame(pt) > 0.2]'

    # Reconstruct the dimuon candidate
    ma.cutAndCopyList('e+:' + skim_label, 'e+:all', fromIP_cut + ' and ' + electronID_cut + ' and ' + theta_cut, path=path)
    ma.cutAndCopyList('mu+:' + skim_label, 'mu+:all', fromIP_cut + ' and ' + muonID_cut, path=path)
    ma.reconstructDecay(emu_name + ' -> e+:' + skim_label + ' mu-:' + skim_label, emu_cut, path=path)

    # And return the dimuon list
    emu_list.append(emu_name)
    return emu_list


def DielectronPlusMissingEnergyList(path):
    """
    Warning:
        This skim is currently deactivated, since the retention rate is too high

    Dielectron skim, needed for :math:`e^{+}e^{-} \\to A^{\prime} h^{\prime};`
    :math:`A^{\prime} \\to e^{+}e^{-}; \, h^{\prime} \\to \mathrm{invisible}` and other searches

    **Skim code**: 18520300

    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}`

    **Skim category**: physics, dark sector

    Parameters:
        path (basf2.Path): the path to add the skim

    Returns:
        list containing the candidate names
    """
    __author__ = 'Giacomo De Pietro'

    # FIXME this skim is currently deactivated: delete the following two lines to activate it
    # and update the Sphinx documentation
    b2.B2WARNING("The skim 'DielectronPlusMissingEnergy' is currently deactivated.")
    return []

    dielectron_list = []
    skim_label = 'forDielectronMissingEnergySkim'
    dielectron_name = 'Z0:' + skim_label

    # Define some basic cuts
    fromIP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    electronID_cut = '[electronID > 0.2]'
    # We require that the electron points to the barrel ECL + 10 degrees
    theta_cut = '[0.387 < theta < 2.421]'
    # We want exactly 2 tracks from IP
    dielectron_cut = '[nCleanedTracks(' + fromIP_cut + ') == 2]'
    # And the pair must have pt > 200 MeV in CMS frame
    dielectron_cut += ' and [useCMSFrame(pt) > 0.2]'

    # Reconstruct the dielectron candidate
    ma.cutAndCopyList('e+:' + skim_label, 'e+:all', fromIP_cut + ' and ' + electronID_cut + ' and ' + theta_cut, path=path)
    ma.reconstructDecay(dielectron_name + ' -> e+:' + skim_label + ' e-:' + skim_label, dielectron_cut, path=path)

    # And return the dielectron list
    dielectron_list.append(dielectron_name)
    return dielectron_list


def TwoTrackLeptonsForLuminosityList(path):
    """
    Skim list for two track (e+e- to e+e- and e+e- to mu+mu-) events.

    **Skim code**: 18530100

    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}` and `e^{+}e^{-} \\to \mu^{+}\mu^{-}`

    **Skim category**: physics, luminosity

    Parameters:
        path (basf2.Path): path to add the skim

    Returns:
        list name of the skim candidates
    """
    __author__ = 'Xing-Yu Zhou'

    skim_label = 'TwoTrackLeptonsForLuminosity'

    # Tracks from IP
    IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    # Tracks of momenta greater than 2 GeV in the CMS frame
    p_cut = '[useCMSFrame(p) > 2.0]'
    # Tracks points to the barrel ECL + 10 degrees
    theta_cut = '[0.387 < theta < 2.421]'
    single_track_cut = IP_cut + ' and ' + p_cut + ' and ' + theta_cut

    # Exactly 2 tracks from IP
    nTracks_cut = '[nCleanedTracks(' + single_track_cut + ') == 2]'
    # Acollinearity angle in the theta dimension less than 10 degrees in the CMS frame
    # candidates are : vpho -> e+ e-
    # daughter indices are:    0  1
    deltaTheta_cut = '[abs(formula(daughter(0, useCMSFrame(theta)) + daughter(1, useCMSFrame(theta)) - 3.1415927)) < 0.17453293]'
    two_track_cut = nTracks_cut + ' and ' + deltaTheta_cut

    # Reconstruct the two track event candidate
    ma.fillParticleList('e+:all', '', path=path)
    ma.cutAndCopyList('e+:' + skim_label, 'e+:all', single_track_cut + ' and ' + nTracks_cut, path=path)
    ma.reconstructDecay('vpho:' + skim_label + ' -> e+:' + skim_label + ' e-:' + skim_label, two_track_cut, path=path)

    return ['vpho:' + skim_label]


def TwoTrackPionsList(path):
    """
    Skim list for two track (e+e- to pi+pi-) events.

    **Skim code**: 18520500

    **Physics channel**: :math:`e^{+}e^{-} \\to \pi^{+}\pi^{-}`

    **Skim category**: physics, dark sector

    Parameters:
        path (basf2.Path): path to add the skim

    Returns:
        list name of the skim candidates
    """
    __author__ = 'Xing-Yu Zhou'

    skim_label = 'TwoTrackPions'

    # Tracks from IP
    IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    # Tracks points to the barrel ECL + 10 degrees
    theta_cut = '[0.387 < theta < 2.421]'
    single_track_cut = IP_cut + ' and ' + theta_cut

    # Exactly 2 tracks
    nTracks_cut = '[nCleanedTracks(' + single_track_cut + ') == 2]'
    # Invariant mass of pi+pi- system less than 1.5 GeV
    M_cut = 'M < 1.5'
    two_track_cut = nTracks_cut + ' and ' + M_cut

    # Reconstruct the two track event candidate
    ma.fillParticleList('pi+:all', '', path=path)
    ma.cutAndCopyList('pi+:' + skim_label, 'pi+:all', single_track_cut + ' and ' + nTracks_cut, path=path)
    ma.reconstructDecay('vpho:' + skim_label + ' -> pi+:' + skim_label + ' pi-:' + skim_label, two_track_cut, path=path)

    return ['vpho:' + skim_label]
