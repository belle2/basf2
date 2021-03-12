#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for the low multiplicity physics working group """

__authors__ = [
    "Xing-Yu Zhou",
    "Hisaki Hayashii"
]


import modularAnalysis as ma
from skimExpertFunctions import BaseSkim, fancy_skim_header, get_test_file
from stdCharged import stdE, stdPi
from stdPhotons import stdPhotons
from variables import variables as vm

_VALIDATION_SAMPLE = "mdst14.root"


@fancy_skim_header
class TwoTrackLeptonsForLuminosity(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}` and :math:`e^{+}e^{-} \\to \\mu^{+}\\mu^{-}`
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
    **Physics channel**: :math:`e^{+}e^{-} \\to \\gamma h_{1}^{+}h_{2}^{-} X`

    .. Note::
        The :math:`h_{1}^{+}` and :math:`h_{2}^{+}` here mean a positive particle
        and a negative particle that could be either conjugate or non-conjugate. The
        :math:`X` means arbitrary final state particles.

    **Decay Modes**

        1. :math:`e^{+}e^{-} \\to \\gamma \\pi^{+} \\pi^{-} X`,
        2. :math:`e^{+}e^{-} \\to \\gamma K^{+} K^{-} X`,
        3. :math:`e^{+}e^{-} \\to \\gamma K^{+} \\pi^{-} X`,
        4. :math:`e^{+}e^{-} \\to \\gamma p \\overline{p} X`,
        5. :math:`e^{+}e^{-} \\to \\gamma p \\pi^{-} X`,
        6. :math:`e^{+}e^{-} \\to \\gamma p K^{-} X`,
    """
    __authors__ = "Xing-Yu Zhou"
    __description__ = "Skim list for low mass events with at least two tracks and one hard photon" \
                      " in final state."
    __contact__ = "Xing-Yu Zhou <xing-yu.zhou@desy.de>"
    __category__ = "physics, low multiplicity"

    TestFiles = [get_test_file("MC13_mumuBGx1"), get_test_file("MC13_uubarBGx1")]
    validation_sample = _VALIDATION_SAMPLE

    def build_lists(self, path):
        label = "LowMassTwoTrack"

        # Momenta of tracks greater than 0.5 GeV in the Lab frame
        pCut = "p > 0.5"
        # Energy of hard ISR gamma greater than 2 GeV in the CMS frame
        ISRECut = "useCMSFrame(E) > 2"
        # Invariant mass of h+h- system less than 3.5 GeV
        hhMassWindow = "daughterInvM(1,2) < 3.5"

        # Event based cut
        # Number of tracks passing the selection criteria, should be greater than or equal to to 2
        nTracksCut = f"nCleanedTracks({pCut}) >= 2"
        # Require at least one hard photon
        nHardISRPhotonCut = f"nCleanedECLClusters({ISRECut}) > 0"

        # Apply event based cuts
        ma.applyEventCuts(f"{nTracksCut} and {nHardISRPhotonCut}", path=path)

        # Reconstruct candidates
        ma.fillParticleList(f"pi+:{label}", pCut, path=path)
        ma.fillParticleList(f"K+:{label}", pCut, path=path)
        ma.fillParticleList(f"p+:{label}", pCut, path=path)
        ma.fillParticleList(f"gamma:{label}_ISR", ISRECut, path=path)

        # the mass hypothesis is different for p+, pi+ and K+ lists, so it is good to write them separately.
        ModesAndCuts = [
            (f"vpho:{label}_pipi", f" -> gamma:{label}_ISR pi+:{label} pi-:{label}", hhMassWindow),
            (f"vpho:{label}_KK", f" -> gamma:{label}_ISR K+:{label} K-:{label}", hhMassWindow),
            # Might be useful when one wants to reconstruct ISR K pi and missing other final state particles
            (f"vpho:{label}_Kpi", f" -> gamma:{label}_ISR K+:{label} pi-:{label}", hhMassWindow),
            (f"vpho:{label}_pp", f" -> gamma:{label}_ISR p+:{label} anti-p-:{label}", hhMassWindow),
            # Useful for analyses for processes like ISR Lambda Lambda-bar (Sigma Sigma-bar) , especially when
            # one wants to reconstruct the hard ISR photon and one of the Lambda (Sigma), missing anthoer
            # Lambda (Sigma)
            (f"vpho:{label}_ppi", f" -> gamma:{label}_ISR p+:{label} pi-:{label}", hhMassWindow),
            # Might be useful when one wants to reconstruct ISR p K and missing other final state particles
            (f"vpho:{label}_pK", f" -> gamma:{label}_ISR p+:{label} K-:{label}", hhMassWindow),
        ]

        self.SkimLists = []
        for dmID, (mode, decayString, cut) in enumerate(ModesAndCuts):
            ma.reconstructDecay(mode + decayString, cut, dmID=dmID, path=path)
            self.SkimLists.append(mode)

    def validation_histograms(self, path):
        vm.addAlias('pip_p_cms', 'daughter(0, useCMSFrame(p))')
        vm.addAlias('pim_p_cms', 'daughter(1, useCMSFrame(p))')
        vm.addAlias('gamma_E_cms', 'daughter(2, useCMSFrame(E))')
        vm.addAlias('pip_theta_lab', 'formula(daughter(0, theta)*180/3.1415927)')
        vm.addAlias('pim_theta_lab', 'formula(daughter(1, theta)*180/3.1415927)')
        vm.addAlias('gamma_theta_lab', 'formula(daughter(2, theta)*180/3.1415927)')
        vm.addAlias('Mpipi', 'daughterInvM(0,1)')

        ma.copyLists('vpho:LowMassTwoTrack', self.SkimLists, path=path)

        variablesHist = [
            ('pip_p_cms', 60, 0, 6),
            ('pim_p_cms', 60, 0, 6),
            ('gamma_E_cms', 60, 0, 6),
            ('pip_theta_lab', 90, 0, 180),
            ('pim_theta_lab', 90, 0, 180),
            ('gamma_theta_lab', 90, 0, 180),
            ('Mpipi', 80, 0., 4.),
            ('M', 60, 6., 12.)
        ]

        # Output the variables to histograms
        ma.variablesToHistogram(
            'vpho:LowMassTwoTrack',
            variablesHist,
            filename=f'{self}_Validation.root',
            path=path)


@fancy_skim_header
class SingleTagPseudoScalar(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to  e^{\\pm} (e^{\\mp}) \\pi^{0}/\\eta/\\eta^{\\prime}`

    **Decay Modes**

        1. :math:`\\pi^{0}\\to \\gamma \\gamma`,
        2. :math:`\\eta \\to \\gamma\\gamma`,
        3. :math:`\\eta \\to \\pi^{+}\\pi^{-}\\pi^{0}`,
        4. :math:`\\eta \\to \\pi^{+}\\pi^{-}\\gamma`,
        5. :math:`\\eta^{\\prime} \\to \\pi^{+}\\pi^{-}\\eta(\\to \\gamma\\gamma)`,
        6. :math:`\\eta^{\\prime} \\to \\pi^{+}\\pi^{-}\\gamma`
    """

    __authors__ = ["Hisaki Hayashii"]
    __contact__ = "Hisaki Hayashii <hisaki.hayashii@desy.de>"
    __description__ = "A skim script to select events with one high-energy electron and one or more pi0/eta/eta mesons."
    __category__ = "physics, low multiplicity"

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):

        label = "PseudoScalarSkim"
        TrackCuts = "abs(dz) < 2.0 and dr < 0.5 and pt > 0.15"

        ma.fillParticleList(f"e+:{label}", f"{TrackCuts} and E > 1.5 and electronID > 0.7", path=path)
        ma.fillParticleList(f"pi+:{label}", f"{TrackCuts} and electronID < 0.7", path=path)
        ma.fillParticleList(f"gamma:{label}", "clusterE > 0.1", path=path)

        pi0MassWindow = "0.06 < InvM < 0.18"
        etaMassWindow = "0.50 < InvM < 0.60"
        etapMassWindow = "0.91 < InvM < 1.10"
        ModesAndCuts = [
            (f"pi0:{label}_loose -> gamma:{label} gamma:{label}", pi0MassWindow),
            (f"eta:gg -> gamma:{label} gamma:{label}", etaMassWindow),
            (f"eta:pipipi0 -> pi+:{label} pi-:{label} pi0:{label}_loose", etaMassWindow),
            (f"eta:pipig -> pi+:{label} pi-:{label} gamma:{label}", etaMassWindow),
            (f"eta':pipieta_gg -> pi+:{label} pi-:{label} eta:gg", etapMassWindow),
            (f"eta':pipig -> pi+:{label} pi-:{label} gamma:{label}", etapMassWindow),
        ]
        for dmID, (mode, cut) in enumerate(ModesAndCuts):
            ma.reconstructDecay(mode, cut, dmID=dmID, path=path)

        ma.cutAndCopyList(f"pi0:{label}_highE", f"pi0:{label}_loose", "E > 0.5", path=path)

        particles = [
            f"pi0:{label}_highE",
            "eta:gg",
            "eta:pipipi0",
            "eta:pipig",
            "eta':pipieta_gg",
            "eta':pipig"
        ]
        ModeSum = " + ".join(f"nParticlesInList({particle})" for particle in particles)
        presel = f"nParticlesInList(e+:{label}) == 1 and nParticlesInList(pi+:{label}) <= 2"
        EventCuts = f"{presel} and formula({ModeSum}) >= 1"

        # Although a condition of "mode_sum >= 1" looks like very loose,
        # the reduction rate of this SingleTagPseudoScalar skim is very large, i.e. 1/50,
        # since the requirements, one high-energy electron and <=2 other charged
        # tracks, are quite stringent.
        path = self.skim_event_cuts(EventCuts, path=path)

        self.SkimLists = [f"e+:{label}"]
