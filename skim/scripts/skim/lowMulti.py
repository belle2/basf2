#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the low multiplicity physics working group """

__authors__ = [
    "Xing-Yu Zhou"
]


import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, get_test_file


def TwoTrackLeptonsForLuminosityList(path):
    """
    Skim list for two track lepton (e+e- to e+e- and e+e- to mu+mu-) events for luminosity measurements.

    **Skim code**: 18530100

    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}` and `e^{+}e^{-} \\to \mu^{+}\mu^{-}`

    **Skim category**: physics, low multiplicity

    Parameters:
        path (basf2.Path): path to add the skim

    Returns:
        list name of the skim candidates
    """
    __author__ = 'Xing-Yu Zhou'

    # Skim label for the case of two tracks
    skim_label_2 = 'TwoTrackLeptonsForLuminosity2'
    # Skim label for the case of one track plus one cluster
    skim_label_1 = 'TwoTrackLeptonsForLuminosity1'

    # Tracks from IP
    IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    # Tracks or clusters of momenta greater than 2 GeV in the CMS frame
    p_cut = '[useCMSFrame(p) > 2.0]'
    # Tracks pointing to or clusters locating in the barrel ECL + 10 degrees
    theta_cut = '[0.387 < theta < 2.421]'

    single_track_cut = IP_cut + ' and ' + p_cut + ' and ' + theta_cut
    single_cluster_cut = p_cut + ' and ' + theta_cut

    # Exactly 2 tracks
    nTracks_cut_2 = '[nCleanedTracks(' + single_track_cut + ') == 2]'
    # Exactly 1 track
    nTracks_cut_1 = '[nCleanedTracks(' + single_track_cut + ') == 1]'
    # Acollinearity angle in the theta dimension less than 10 degrees in the CMS frame
    # candidates are : vpho -> e+ e- or vpho -> e gamma
    # daughter indices are:    0  1             0 1
    deltaTheta_cut = '[abs(formula(daughter(0, useCMSFrame(theta)) + daughter(1, useCMSFrame(theta)) - 3.1415927)) < 0.17453293]'

    two_track_cut = nTracks_cut_2 + ' and ' + deltaTheta_cut
    track_cluster_cut = nTracks_cut_1 + ' and ' + deltaTheta_cut

    # Reconstruct the event candidates with two tracks
    ma.fillParticleList('e+:' + skim_label_2, single_track_cut + ' and ' + nTracks_cut_2, path=path)
    ma.reconstructDecay('vpho:' + skim_label_2 + ' -> e+:' + skim_label_2 + ' e-:' + skim_label_2, two_track_cut, path=path)

    # Reconstruct the event candidates with one track plus one cluster
    ma.fillParticleList('e+:' + skim_label_1, single_track_cut + ' and ' + nTracks_cut_1, path=path)
    ma.fillParticleList('gamma:' + skim_label_1, single_cluster_cut + ' and ' + nTracks_cut_1, path=path)
    ma.reconstructDecay(
        'vpho:' +
        skim_label_1 +
        ' -> e+:' +
        skim_label_1 +
        ' gamma:' +
        skim_label_1,
        track_cluster_cut,
        allowChargeViolation=True,
        path=path)

    return ['vpho:' + skim_label_2, 'vpho:' + skim_label_1]


def LowMassTwoTrackList(path):
    """
    Skim list for low mass two track events.

    **Skim code**: 18530200

    **Physics channel**: :math:`e^{+}e^{-} \\to \gamma h^{+}h^{-}`

    **Skim category**: physics, low multiplicity

    Parameters:
        path (basf2.Path): path to add the skim

    Returns:
        list name of the skim candidates
    """
    __author__ = 'Xing-Yu Zhou'

    skim_label = 'LowMassTwoTrack'

    # Tracks from IP
    IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
    # Clusters of energy greater than 2 GeV in the CMS frame
    E_cut = '[useCMSFrame(E) > 2]'

    # Number of cleaned tracks larger than 1 and less than 5
    nTracks_cut = '[nCleanedTracks(' + IP_cut + ') > 1] and [nCleanedTracks(' + IP_cut + ') < 5]'
    # Exactly 1 cleaned cluster
    nClusters_cut_1ISR = '[nCleanedECLClusters(' + E_cut + ') == 1]'
    # Exactly 2 cleaned clusters
    nClusters_cut_2ISR = '[nCleanedECLClusters(' + E_cut + ') == 2]'

    # Invariant mass of pi+pi- system less than 3.5 GeV
    Mpipi_cut = 'daughterInvM(0,1) < 3.5'
    # Invariant mass of pi+pi-ISR system larger than 5 GeV
    M_cut = 'M > 5'
    two_track_cut = Mpipi_cut + ' and ' + M_cut

    # Reconstruct the two track event candidate
    ma.fillParticleList(
        'pi+:' + skim_label,
        IP_cut + ' and ' + nTracks_cut + ' and [' + nClusters_cut_1ISR + ' or ' + nClusters_cut_2ISR + ']',
        path=path)
    ma.fillParticleList(
        'gamma:' + skim_label,
        E_cut + ' and ' + nTracks_cut + ' and [' + nClusters_cut_1ISR + ' or ' + nClusters_cut_2ISR + ']',
        path=path)
    ma.reconstructDecay(
        'vpho:' + skim_label + '1ISR -> pi+:' + skim_label + ' pi-:' + skim_label + ' gamma:' + skim_label,
        nClusters_cut_1ISR + ' and ' + two_track_cut,
        path=path)
    ma.reconstructDecay(
        'vpho:' + skim_label + '2ISR -> pi+:' + skim_label + ' pi-:' + skim_label + ' gamma:' + skim_label + ' gamma:' + skim_label,
        nClusters_cut_2ISR + ' and ' + two_track_cut,
        path=path)
    ma.copyLists(
        'vpho:' + skim_label,
        ['vpho:' + skim_label + '1ISR', 'vpho:' + skim_label + '2ISR'],
        path=path)

    return ['vpho:' + skim_label]


class TwoTrackLeptonsForLuminosity(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}` and `e^{+}e^{-} \\to \mu^{+}\mu^{-}`
    """
    __authors__ = "Xing-Yu Zhou"
    __description__ = "Skim list for two track lepton (e+e- to e+e- and e+e- to mu+mu-) events for luminosity measurements."
    __contact__ = "Xing-Yu Zhou <xing-yu.zhou@desy.de>"
    __category__ = "physics, low multiplicity"

    RequiredStandardLists = None
    TestFiles = [get_test_file("MC13_mumuBGx1")]

    def build_lists(self, path):
        # Skim label for the case of two tracks
        skim_label_2 = 'TwoTrackLeptonsForLuminosity2'
        # Skim label for the case of one track plus one cluster
        skim_label_1 = 'TwoTrackLeptonsForLuminosity1'

        # Tracks from IP
        IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
        # Tracks or clusters of momenta greater than 2 GeV in the CMS frame
        p_cut = '[useCMSFrame(p) > 2.0]'
        # Tracks pointing to or clusters locating in the barrel ECL + 10 degrees
        theta_cut = '[0.387 < theta < 2.421]'

        single_track_cut = IP_cut + ' and ' + p_cut + ' and ' + theta_cut
        single_cluster_cut = p_cut + ' and ' + theta_cut

        # Exactly 2 tracks
        nTracks_cut_2 = '[nCleanedTracks(' + single_track_cut + ') == 2]'
        # Exactly 1 track
        nTracks_cut_1 = '[nCleanedTracks(' + single_track_cut + ') == 1]'
        # Acollinearity angle in the theta dimension less than 10 degrees in the CMS frame
        # candidates are : vpho -> e+ e- or vpho -> e gamma
        # daughter indices are:    0  1             0 1
        deltaTheta_cut = (
            '[abs(formula(daughter(0, useCMSFrame(theta)) + daughter(1, useCMSFrame(theta)) - 3.1415927)) < 0.17453293]'
        )

        two_track_cut = nTracks_cut_2 + ' and ' + deltaTheta_cut
        track_cluster_cut = nTracks_cut_1 + ' and ' + deltaTheta_cut

        # Reconstruct the event candidates with two tracks
        ma.fillParticleList('e+:' + skim_label_2, single_track_cut + ' and ' + nTracks_cut_2, path=path)
        ma.reconstructDecay('vpho:' + skim_label_2 + ' -> e+:' + skim_label_2 + ' e-:' + skim_label_2, two_track_cut, path=path)

        # Reconstruct the event candidates with one track plus one cluster
        ma.fillParticleList('e+:' + skim_label_1, single_track_cut + ' and ' + nTracks_cut_1, path=path)
        ma.fillParticleList('gamma:' + skim_label_1, single_cluster_cut + ' and ' + nTracks_cut_1, path=path)
        ma.reconstructDecay(
            'vpho:' +
            skim_label_1 +
            ' -> e+:' +
            skim_label_1 +
            ' gamma:' +
            skim_label_1,
            track_cluster_cut,
            allowChargeViolation=True,
            path=path)

        self.SkimLists = ['vpho:' + skim_label_2, 'vpho:' + skim_label_1]


class LowMassTwoTrack(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to \gamma h^{+}h^{-}`
    """
    __authors__ = "Xing-Yu Zhou"
    __description__ = "Skim list for low mass two track events."
    __contact__ = "Xing-Yu Zhou <xing-yu.zhou@desy.de>"
    __category__ = "physics, low multiplicity"

    RequiredStandardLists = None
    TestFiles = [get_test_file("MC13_mumuBGx1")]

    def build_lists(self, path):
        skim_label = 'LowMassTwoTrack'

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
        ma.fillParticleList('pi+:' + skim_label, single_track_cut + ' and ' + nTracks_cut, path=path)
        ma.reconstructDecay('vpho:' + skim_label + ' -> pi+:' + skim_label + ' pi-:' + skim_label, two_track_cut, path=path)

        self.SkimLists = ['vpho:' + skim_label]
