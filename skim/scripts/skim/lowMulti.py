#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the low multiplicity physics working group """

__authors__ = [
    "Xing-Yu Zhou"
]


import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header, get_test_file


@fancy_skim_header
class TwoTrackLeptonsForLuminosity(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}` and `e^{+}e^{-} \\to \mu^{+}\mu^{-}`
    """
    __authors__ = "Xing-Yu Zhou"
    __description__ = "Skim list for two track lepton (e+e- to e+e- and e+e- to mu+mu-) events for luminosity measurements."
    __contact__ = "Xing-Yu Zhou <xing-yu.zhou@desy.de>"
    __category__ = "physics, low multiplicity"

    TestFiles = [get_test_file("MC13_mumuBGx1")]

    def __init__(self, prescale=1, **kwargs):
        """
        Parameters:
            prescale (int): the prescale for this skim
            **kwargs: Passed to the constructor of `BaseSkim`
        """
        # Redefine __init__ to allow for additional optional arguments
        super().__init__(**kwargs)
        self.prescale = prescale

    def build_lists(self, path):
        # Skim label
        skim_label = 'TwoTrackLeptonsForLuminosity'
        # Skim label for the case of two tracks
        skim_label_2 = 'TwoTrackLeptonsForLuminosity2'
        # Skim label for the case of one track plus one cluster
        skim_label_1 = 'TwoTrackLeptonsForLuminosity1'

        # Tracks from IP
        IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
        # Tracks or clusters of momenta greater than 2 GeV in the CMS frame
        p_cut = '[useCMSFrame(p) > 2.0]'
        # Tracks pointing to or clusters locating in the barrel ECL + 10 degrees
        theta_cut = '[0.561 < theta < 2.247]'

        single_track_cut = IP_cut + ' and ' + p_cut + ' and ' + theta_cut
        single_cluster_cut = p_cut + ' and ' + theta_cut

        # Exactly 2 tracks
        nTracks_cut_2 = '[nCleanedTracks(' + single_track_cut + ') == 2 or nCleanedTracks(' + single_track_cut + ') == 3]'
        # Exactly 1 track
        nTracks_cut_1 = '[nCleanedTracks(' + single_track_cut + ') == 1]'
        # Acollinearity angle in the theta dimension less than 10 degrees in the CMS frame
        # candidates are : vpho -> e+ e- or vpho -> e gamma
        # daughter indices are:    0  1             0 1
        deltaTheta_cut = (
            '[abs(formula(daughter(0, useCMSFrame(theta)) + daughter(1, useCMSFrame(theta)) - 3.1415927)) < 0.17453293]'
        )

        # convert the prescale from trigger convention
        prescale = str(float(1.0 / self.prescale))
        prescale_logic = 'eventRandom <= ' + prescale

        two_track_cut = nTracks_cut_2 + ' and ' + deltaTheta_cut + ' and ' + prescale_logic
        track_cluster_cut = nTracks_cut_1 + ' and ' + deltaTheta_cut + ' and ' + prescale_logic

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

        ma.copyLists('vpho:' + skim_label, ['vpho:' + skim_label_2, 'vpho:' + skim_label_1], path=path)
        self.SkimLists = ['vpho:' + skim_label]


@fancy_skim_header
class LowMassTwoTrack(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to \gamma h^{+}h^{-}`
    """
    __authors__ = "Xing-Yu Zhou"
    __description__ = "Skim list for low mass two track events."
    __contact__ = "Xing-Yu Zhou <xing-yu.zhou@desy.de>"
    __category__ = "physics, low multiplicity"

    TestFiles = [get_test_file("MC13_mumuBGx1")]

    def build_lists(self, path):
        skim_label = 'LowMassTwoTrack'

        # Tracks from IP
        IP_cut = '[abs(dz) < 5.0] and [abs(dr) < 2.0]'
        # Tracks of momenta greater than 0.5 GeV in the Lab frame
        p_cut = '[p > 0.5]'
        # Clusters of energy greater than 2 GeV in the CMS frame
        E_cut = '[useCMSFrame(E) > 2]'
        # Number of cleaned tracks larger than 1 and less than 5
        nTracks_cut = '[nCleanedTracks(' + IP_cut + ' and ' + p_cut + ')] == 2'
        # Invariant mass of pi+pi- system less than 3.5 GeV
        Mpipi_cut = 'daughterInvM(0,1) < 3.5'

        # Reconstruct the two track event candidate
        ma.fillParticleList('pi+:' + skim_label, IP_cut + ' and ' + p_cut + ' and ' + nTracks_cut, path=path)
        ma.fillParticleList('gamma:' + skim_label, E_cut + ' and ' + nTracks_cut, path=path)
        ma.reconstructDecay(
            'vpho:' +
            skim_label +
            ' -> pi+:' +
            skim_label +
            ' pi-:' +
            skim_label +
            ' gamma:' +
            skim_label,
            Mpipi_cut,
            path=path)

        self.SkimLists = ['vpho:' + skim_label]
