#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the dark sector physics working group """

__authors__ = [
    "Sam Cunliffe",
    "Michael De Nuccio",
    "Ilya Komarov",
    "Giacomo De Pietro",
    "Miho Wakai",
    "Xing-Yu Zhou"
]


import basf2 as b2
import pdg
import modularAnalysis as ma
from skimExpertFunctions import ifEventPasses, BaseSkim, fancy_skim_header, get_test_file


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


def GammaGammaControlKLMDarkList(path, prescale_high=1, prescale_low=1):
    """
    Gamma gamma skim list for study of the KLM efficiency as part of
    the dark photon analysis

    **Skim code**: 18020200

    **Physics channel**: ee → γγ

    **Skim category**: physics, dark sector, control-channel

    Info:
        This skim can retain a lot of γγ events.
        In case this becomes unacceptable, we provide prescale parameters.
        Prescales are given in standard trigger convention (reciprocal),
        so prescale of 100 is 1% of events kept, etc.

    Example:
        To prescale the higher-energy probe photons by 10%:

        >>> GammaGammaControlKLMDarkList(path=mypath, prescale_high=10)

    Parameters:
        path (basf2.Path): the path to add the skim
        prescale_high (int): the prescale for more energetic probe photon
        prescale_low (int): the prescale for a less energetic probe photon

    Returns:
        list name of the skim candidates
    """
    __authors__ = ["Sam Cunliffe", "Miho Wakai"]

    # unpack prescales and convert from trigger convention to a number we can
    # compare with a float
    if (prescale_high, prescale_low) is not (1, 1):
        b2.B2INFO('GammaGammaControlKLMDarkList is prescaled. prescale_high=%i, prescale_low=%i' % (prescale_high, prescale_low))
    prescale_high = str(float(1.0 / prescale_high))
    prescale_low = str(float(1.0 / prescale_low))

    # no good (IP-originating) tracks in the event
    good_tracks = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.2'  # cm, cm, GeV/c
    no_good_tracks = 'nCleanedTracks(' + good_tracks + ') < 1'

    # get two most energetic photons in the event (must be at least 100 MeV
    # and not more than 7 GeV)
    ma.cutAndCopyList(
        'gamma:controlKLM', 'gamma:all', '0.1 < useCMSFrame(clusterE) < 7', path=path)
    ma.rankByHighest('gamma:controlKLM', 'useCMSFrame(clusterE)', numBest=2, path=path)

    # will build pairwise candidates from the gamma:controlKLM list:
    # vpho -> gamma gamma

    # the more energetic must be at least 4.5 GeV
    tag_daughter = 'daughterHighest(useCMSFrame(clusterE)) > 4.5'
    # note that sometimes the probe will also fulfill this criteria, but the
    # candidate list will *not* be double-counted: these extra candidates need
    # to be added back offline

    # apply prescales to the less energetic daughter: compare to the eventwise random number
    probe_high = '[daughterLowest(useCMSFrame(clusterE)) > 4.5] and [eventRandom < %s]' % prescale_high
    probe_low = '[daughterLowest(useCMSFrame(clusterE)) < 4.5] and [eventRandom < %s]' % prescale_low
    prescale = '[ %s ] or [ %s ]' % (probe_high, probe_low)

    # ~back-to-back in phi in the CMS (3.1066... radians = 178 degrees)
    delta_phi_cut = 'daughterDiffOfPhiCMS(0, 1) > 3.1066860685499065'

    # sum theta in the cms 178 --> 182 degrees
    sum_th = 'daughterSumOf(useCMSFrame(theta))'
    sum_th_cut = '3.1066860685499065 < ' + sum_th + ' < 3.1764992386296798'

    # now build and return the candidates passing the AND of our cuts
    cuts = '[ %s ]' % no_good_tracks
    cuts += ' and [ %s ]' % tag_daughter
    cuts += ' and [ %s ]' % prescale
    cuts += ' and [ %s ]' % delta_phi_cut
    cuts += ' and [ %s ]' % sum_th_cut
    ma.reconstructDecay(
        'vpho:singlePhotonControlKLM -> gamma:controlKLM gamma:controlKLM',
        cuts, path=path)
    return ['vpho:singlePhotonControlKLM']


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
    needed for :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-} Z^{\\prime}; \\, Z^{\\prime} \\to \\mathrm{invisible}` and other searches

    **Skim code**: 18520100

    **Physics channel**: :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-} \\, +` missing energy

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
    needed for :math:`e^{+}e^{-} \\to e^{\\pm}\\mu^{\\mp} Z^{\\prime}; \\, Z^{\\prime} \\to \\mathrm{invisible}` and other searches

    **Skim code**: 18520200

    **Physics channel**: :math:`e^{+}e^{-} \\to e^{\\pm}\\mu^{\\mp} \\, +` missing energy

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

    Dielectron skim, needed for :math:`e^{+}e^{-} \\to A^{\\prime} h^{\\prime};`
    :math:`A^{\\prime} \\to e^{+}e^{-}; \\, h^{\\prime} \\to \\mathrm{invisible}` and other searches

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
    M_cut = 'M < 3.5'
    two_track_cut = nTracks_cut + ' and ' + M_cut

    # Reconstruct the two track event candidate
    ma.fillParticleList('pi+:all', '', path=path)
    ma.cutAndCopyList('pi+:' + skim_label, 'pi+:all', single_track_cut + ' and ' + nTracks_cut, path=path)
    ma.reconstructDecay('vpho:' + skim_label + ' -> pi+:' + skim_label + ' pi-:' + skim_label, two_track_cut, path=path)

    return ['vpho:' + skim_label]


@fancy_skim_header
class SinglePhotonDark(BaseSkim):
    """
    **Physics channel**: ee → A'γ; A' → invisible

    Skim list contains single photon candidates for the dark photon to invisible final
    state analysis.
    """
    __authors__ = ["Sam Cunliffe"]
    __contact__ = "Sam Cunliffe <sam.cunliffe@desy.de>"
    __description__ = "Single photon skim list for the dark photon analysis."
    __category__ = "physics, dark sector"

    RequiredStandardLists = {
        "stdPhotons": {
            "stdPhotons": ["all"],
        },
    }

    def build_lists(self, path):
        # no good tracks in the event
        cleaned = 'abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.15'  # cm, cm, GeV/c

        # no other photon above 100 MeV
        angle = "0.296706 < theta < 2.61799"  # rad, (17 -- 150 deg)
        minimum = "E > 0.1"  # GeV
        ma.cutAndCopyList("gamma:100", "gamma:all", minimum + " and " + angle, path=path)

        path = self.skim_event_cuts(
            f"0 < nParticlesInList(gamma:100) <  2 and nCleanedTracks({cleaned}) < 1",
            path=path
        )

        # all remaining single photon events (== candidates) with region
        # dependent minimum energy in GeV
        region_dependent = " [clusterReg ==  2 and useCMSFrame(E) > 1.0] or "  # barrel
        region_dependent += "[clusterReg ==  1 and useCMSFrame(E) > 2.0] or "  # fwd
        region_dependent += "[clusterReg ==  3 and useCMSFrame(E) > 2.0] or "  # bwd
        region_dependent += "[clusterReg == 11 and useCMSFrame(E) > 2.0] or "  # between fwd and barrel
        region_dependent += "[clusterReg == 13 and useCMSFrame(E) > 2.0] "     # between bwd and barrel
        ma.cutAndCopyList("gamma:singlePhoton", "gamma:100", region_dependent, path=path)
        self.SkimLists = ["gamma:singlePhoton"]


@fancy_skim_header
class ALP3Gamma(BaseSkim):
    __authors__ = ["Michael De Nuccio"]
    __description__ = (
        "Neutral dark sector skim list for the ALP 3-photon analysis: "
        ":math:`ee\\to a(\\to\\gamma\\gamma)\\gamma`"
    )
    __contact__ = ""
    __category__ = "physics, dark sector"

    RequiredStandardLists = None

    def addALPToPDG(self):
        """Adds the ALP codes to the basf2 pdg instance """
        pdg.add_particle('beam', 55, 999., 999., 0, 0)
        pdg.add_particle('ALP', 9000006, 999., 999., 0, 0)

    def initialALP(self, path):
        """
        An list builder function for the ALP decays. Part of the `ALP3Gamma` skim.

        Parameters:
            path (basf2.Path): the path to add the skim

        Returns:
            list name of the ALP decays candidates
        """
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

    def additional_setup(self, path):
        self.addALPToPDG()

    def build_lists(self, path):
        # applying invariant mass cut on the beam particle
        beamcuts = "InvM >= formula(0.8 * Ecms) and InvM <= formula(1.05 * Ecms) and maxWeightedDistanceFromAverageECLTime <= 2"

        ALPList = self.initialALP(path=path)

        # applying a lab frame energy cut to the recoil photon
        ma.fillParticleList("gamma:minimumEnergy", "E >= 0.1", True, path=path)
        beamList = []

        # reconstructing decay using the reconstructed ALP
        # from previous function and adding the recoil photon
        for chID, channel in enumerate(ALPList):
            mode = "beam:" + str(chID) + " -> gamma:minimumEnergy " + channel
            print(mode)
            ma.reconstructDecay(mode, beamcuts, chID, path=path)
            beamList.append("beam:" + str(chID))
        self.SkimLists = beamList


@fancy_skim_header
class DimuonPlusMissingEnergy(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-} \\, +` missing energy.
    """
    __authors__ = ["Giacomo De Pietro"]
    __description__ = (
        "Dimuon + missing energy skim, needed for :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-}"
        "Z^{\\prime}; \\, Z^{\\prime} \\to \\mathrm{invisible}` and other searches."
    )
    __contact__ = ""
    __category__ = "physics, dark sector"

    RequiredStandardLists = {
        "stdCharged": {
            "stdMu": ["all"],
        },
    }

    def build_lists(self, path):
        dimuon_list = []
        skim_label = "forDimuonMissingEnergySkim"
        dimuon_name = "Z0:" + skim_label

        # Define some cuts
        fromIP_cut = "[abs(dz) < 5.0] and [abs(dr) < 2.0]"
        muonID_cut = "[muonID > 0.3]"
        # We want exactly 2 tracks from IP
        dimuon_cut = "[nCleanedTracks(" + fromIP_cut + ") < 4]"
        # And the pair must have pt > 200 MeV in CMS frame
        dimuon_cut += " and [useCMSFrame(pt) > 0.2]"

        # Reconstruct the dimuon candidate
        ma.cutAndCopyList("mu+:" + skim_label, "mu+:all", fromIP_cut + " and " + muonID_cut, path=path)
        ma.reconstructDecay(dimuon_name + " -> mu+:" + skim_label + " mu-:" + skim_label, dimuon_cut, path=path)

        # And return the dimuon list
        dimuon_list.append(dimuon_name)
        self.SkimLists = dimuon_list


@fancy_skim_header
class ElectronMuonPlusMissingEnergy(BaseSkim):
    __authors__ = ["Giacomo De Pietro"]
    __description__ = (
        "Electron-muon pair + missing energy skim, needed for :math:`e^{+}e^{-} \\to "
        "e^{\\pm}\\mu^{\\mp} Z^{\\prime}; \\, Z^{\\prime} \\to \\mathrm{invisible}` and other "
        "searches."
    )
    __contact__ = ""
    __category__ = "physics, dark sector"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
            "stdMu": ["all"],
        },
    }

    def build_lists(self, path):
        """
        **Physics channel**: :math:`e^{+}e^{-} \\to e^{\\pm}\\mu^{\\mp} \\, +` missing energy
        """
        emu_list = []
        skim_label = "forElectronMuonMissingEnergySkim"
        emu_name = "Z0:" + skim_label

        # Define some basic cuts
        fromIP_cut = "[abs(dz) < 5.0] and [abs(dr) < 2.0]"
        electronID_cut = "[electronID > 0.3]"
        muonID_cut = "[muonID > 0.3]"
        # We require that the electron points to the barrel ECL + 10 degrees
        theta_cut = "[0.387 < theta < 2.421]"
        # We want exactly 2 tracks from IP
        emu_cut = "[nCleanedTracks(" + fromIP_cut + ") < 4]"
        # And the pair must have pt > 200 MeV in CMS frame
        emu_cut += " and [useCMSFrame(pt) > 0.2]"

        # Reconstruct the dimuon candidate
        ma.cutAndCopyList("e+:" + skim_label, "e+:all", fromIP_cut + " and " + electronID_cut + " and " + theta_cut, path=path)
        ma.cutAndCopyList("mu+:" + skim_label, "mu+:all", fromIP_cut + " and " + muonID_cut, path=path)
        ma.reconstructDecay(emu_name + " -> e+:" + skim_label + " mu-:" + skim_label, emu_cut, path=path)

        # And return the dimuon list
        emu_list.append(emu_name)
        self.SkimLists = emu_list


@fancy_skim_header
class LFVZpVisible(BaseSkim):
    __authors__ = ["Ilya Komarov"]
    __description__ = "Lepton flavour violating Z' skim, Z' to visible FS."
    __contact__ = ""
    __category__ = "physics, dark sector"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all", "loose"],
        },
    }

    def build_lists(self, path):
        """
        **Physics channel**: ee --> e mu Z'; Z' --> e mu
        """
        lfvzp_list = []

        # Here we just want four gpood tracks to be reconstructed
        track_cuts = "abs(dz) < 2.0 and abs(dr) < 0.5"
        Event_cuts_vis = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 4"

        ma.cutAndCopyList("e+:lfvzp", "e+:all", track_cuts, path=path)

        # Z' to lfv: fully reconstructed
        LFVZpVisChannel = "e+:lfvzp e+:lfvzp e-:lfvzp e-:lfvzp"

        ma.reconstructDecay("vpho:vislfvzp -> " + LFVZpVisChannel, Event_cuts_vis, path=path)

        lfvzp_list.append("vpho:vislfvzp")

        # Z' to lfv: part reco
        LFVZpVisChannel = "e+:lfvzp e+:lfvzp e-:lfvzp"
        Event_cuts_vis = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 3"

        ma.reconstructDecay("vpho:3tr_vislfvzp -> " + LFVZpVisChannel, Event_cuts_vis, path=path, allowChargeViolation=True)

        lfvzp_list.append("vpho:3tr_vislfvzp")

        # Z' to lfv: two same-sign tracks
        LFVZpVisChannel = "e+:lfvzp e+:lfvzp"
        Event_cuts_vis = "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5) == 2"
        ma.reconstructDecay("vpho:2tr_vislfvzp -> " + LFVZpVisChannel, Event_cuts_vis, path=path, allowChargeViolation=True)

        lfvzp_list.append("vpho:2tr_vislfvzp")

        self.SkimLists = lfvzp_list


@fancy_skim_header
class GammaGammaControlKLMDark(BaseSkim):
    """
    **Physics channel**: ee → γγ

    .. Note::
        This skim can retain a lot of γγ events.
        In case this becomes unacceptable, we provide prescale parameters.
        Prescales are given in standard trigger convention (reciprocal),
        so prescale of 100 is 1% of events kept, etc.

    .. Tip::
        To prescale the higher-energy probe photons by 10%:

        >>> from skim.dark import GammaGammaControlKLMDark
        >>> Skim = GammaGammaControlKLMDark(prescale_high=10)
        >>> Skim(path)  # Add list-building function and uDST output module to path
        >>> b2.process(path)
    """

    __authors__ = ["Sam Cunliffe", "Miho Wakai"]
    __description__ = (
        "Gamma gamma skim list for study of the KLM efficiency as part of "
        "the dark photon analysis"
    )
    __contact__ = "Sam Cunliffe <sam.cunliffe@desy.de>"
    __category__ = "physics, dark sector, control-channel"

    RequiredStandardLists = {
        "stdPhotons": {
            "stdPhotons": ["all"],
        },
    }

    TestFile = get_test_file("MC13_ggBGx1")

    def __init__(self, prescale_high=1, prescale_low=1, **kwargs):
        """
        Parameters:
            prescale_high (int): the prescale for more energetic probe photon
            prescale_low (int): the prescale for a less energetic probe photon
            **kwargs: Passed to constructor of `BaseSkim`.
        """
        # Redefine __init__ to allow for additional optional arguments
        super().__init__(**kwargs)
        self.prescale_high = prescale_high
        self.prescale_low = prescale_low

    def build_lists(self, path):
        # unpack prescales and convert from trigger convention to a number we can
        # compare with a float
        prescale_high, prescale_low = self.prescale_high, self.prescale_low
        if (prescale_high, prescale_low) is not (1, 1):
            b2.B2INFO(
                "GammaGammaControlKLMDarkList is prescaled. "
                f"prescale_high={prescale_high}, prescale_low={prescale_low}"
            )
        prescale_high = str(float(1.0 / prescale_high))
        prescale_low = str(float(1.0 / prescale_low))

        # no good (IP-originating) tracks in the event
        good_tracks = "abs(dz) < 2.0 and abs(dr) < 0.5 and pt > 0.2"  # cm, cm, GeV/c
        no_good_tracks = f"nCleanedTracks({good_tracks}) < 1"

        # get two most energetic photons in the event (must be at least 100 MeV
        # and not more than 7 GeV)
        ma.cutAndCopyList(
            "gamma:controlKLM", "gamma:all", "0.1 < useCMSFrame(clusterE) < 7", path=path)
        ma.rankByHighest("gamma:controlKLM", "useCMSFrame(clusterE)", numBest=2, path=path)

        # will build pairwise candidates from the gamma:controlKLM list:
        # vpho -> gamma gamma

        # the more energetic must be at least 4.5 GeV
        tag_daughter = "daughterHighest(useCMSFrame(clusterE)) > 4.5"
        # note that sometimes the probe will also fulfill this criteria, but the
        # candidate list will *not* be double-counted: these extra candidates need
        # to be added back offline

        # apply prescales to the less energetic daughter: compare to the eventwise random number
        probe_high = f"[daughterLowest(useCMSFrame(clusterE)) > 4.5] and [eventRandom < {prescale_high}]"
        probe_low = f"[daughterLowest(useCMSFrame(clusterE)) < 4.5] and [eventRandom < {prescale_low}]"
        prescale = f"[ {probe_high} ] or [ {probe_low} ]"

        # ~back-to-back in phi in the CMS (3.1066... radians = 178 degrees)
        delta_phi_cut = "daughterDiffOfPhiCMS(0, 1) > 3.1066860685499065"

        # sum theta in the cms 178 --> 182 degrees
        sum_th = "daughterSumOf(useCMSFrame(theta))"
        sum_th_cut = f"3.1066860685499065 < {sum_th} < 3.1764992386296798"

        # now build and return the candidates passing the AND of our cuts
        cuts = [no_good_tracks, tag_daughter, prescale, delta_phi_cut, sum_th_cut]
        cuts = " and ".join([f"[ {cut} ]" for cut in cuts])

        ma.reconstructDecay(
            "vpho:singlePhotonControlKLM -> gamma:controlKLM gamma:controlKLM",
            cuts, path=path)
        self.SkimLists = ["vpho:singlePhotonControlKLM"]


@fancy_skim_header
class DielectronPlusMissingEnergy(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}`

    Warning:
        This skim is currently deactivated, since the retention rate is too high.
    """

    __authors__ = "Giacomo De Pietro"
    __description__ = (
        "Dielectron skim, needed for :math:`e^{+}e^{-} \\to A^{\\prime} h^{\\prime};`"
        ":math:`A^{\\prime} \\to e^{+}e^{-}; \\, h^{\\prime} \\to \\mathrm{invisible}` and other searches."
    )
    __contact__ = ""
    __category__ = "physics, dark sector"

    RequiredStandardLists = {
        "stdCharged": {
            "stdE": ["all"],
        },
    }

    TestFile = get_test_file("MC13_mumuBGx1")

    def build_lists(self, path):
        dielectron_list = []
        skim_label = "forDielectronMissingEnergySkim"
        dielectron_name = f"Z0:{skim_label}"

        # Define some basic cuts
        fromIP_cut = "[abs(dz) < 5.0] and [abs(dr) < 2.0]"
        electronID_cut = "[electronID > 0.2]"
        # We require that the electron points to the barrel ECL + 10 degrees
        theta_cut = "[0.387 < theta < 2.421]"
        # We want exactly 2 tracks from IP
        dielectron_cut = f"[nCleanedTracks({fromIP_cut}) == 2]"
        # And the pair must have pt > 200 MeV in CMS frame
        dielectron_cut += " and [useCMSFrame(pt) > 0.2]"

        # Reconstruct the dielectron candidate
        electron_cuts = " and ".join([fromIP_cut, electronID_cut, theta_cut])
        ma.cutAndCopyList(f"e+:{skim_label}", "e+:all", electron_cuts, path=path)
        ma.reconstructDecay(f"{dielectron_name} -> e+:{skim_label} e-:{skim_label}", dielectron_cut, path=path)

        # And return the dielectron list
        dielectron_list.append(dielectron_name)
        self.SkimLists = dielectron_list
