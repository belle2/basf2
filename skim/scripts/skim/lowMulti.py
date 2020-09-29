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
from variables import variables as va


@fancy_skim_header
class TwoTrackLeptonsForLuminosity(BaseSkim):
    """
    **Physics channel**: :math:`e^{+}e^{-} \\to e^{+}e^{-}` and :math:`e^{+}e^{-} \\to \mu^{+}\mu^{-}`
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
        IP_cut = '[abs(dz) < 5] and [abs(dr) < 2.0]'
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


@fancy_skim_header
class SingleTagPseudoScalar(BaseSkim):
    """
    **Physics channel**:
    :math:`e^{+}e^{-} \\to  e^{\\pm} (e^{\\mp}) \\pi^{0}/\\eta/\\eta^{\prime}`

    """
    __authors__ = ["Hisaki Hayashii"]
    __contact__ = "Hisaki Hayashii <hisaki.hayashii@desy.de>"
    __description__ = "A skim script to select events with one high-energy electron and one or more pi0/eta/eta mesons."
    __category__ = "physics, low multiplicity"
    """
    **Decay Modes**

    *1      :math:`\\pi^{0}\\to \\gamma \gamma `,
    *2      :math:`\\eta \\to \gamma\\gamma `,
    *3      :math:`\\eta \\to \\pi^{+}\\pi^{-}\\pi^{0}`,
    *4      :math:`\\eta \\to \\pi^{+}\\pi^{-}\\gamma`,
    *5      :math:`\\eta^{\\prime} \\to \\pi^{+}\\pi^{-}\\eta(\\to \gamma\gamma)`,
    *6      :math:`\\eta^{\\prime} \\to \\pi^{+}\\pi^{-}\\gamma`,

    **Additional Cuts**
    """

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):
        # ------------------------------------------------------------
        # Used cuts
        # --
        # Condition for a tagged electron
        #  - The tagged electron must be identified as electron and must have high
        #    energy  greater than 1.5 GeV.
        #  - dz, dr cuts should be tight enough to remove duplicated tracks.
        # --
        Electron_IPcut = 'abs(dz) < 2.0 and dr < 0.5'
        Pt_cut = 'pt > 0.15'
        ElectronIDcut = 'electronID > 0.7'    # require electron ID
        ElectronEcut = 'E > 1.5'              # tagged electron condition.
        # --
        # Condition for charged pions. These pions are used to reconstruct  eta, eta'.
        # -
        Pion_IPcut = 'abs(dz) < 2.0 and dr < 0.5'
        PionIDcut = 'electronID < 0.7'
        # --
        # Condition for photons, used for pi0/eta/eta' reconstruction.
        # -
        good_cluster = 'clusterE > 0.1'
        # --
        # pi0  mass region, A wide mass region is chosen to see background shape.
        # -
        pi0_mass_wide = '0.06 < InvM < 0.18'
        # --
        # pi0_energy.  This energy cut is applied for e(e)pi0 mode only.
        # -
        pi0_energy = ' E > 0.5'
        # --
        # eta, eta'  mass region.
        # A wide mass region is chosen to see the background shape.
        # -
        eta_mass_wide = '0.50 < InvM < 0.60'
        etap_mass_wide = '0.91 < InvM < 1.10'
        # --------------------------------------------------------------------------------------------
        # ---
        #  Selection of tagged electron/positron.
        #
        #   particle-list:
        #    e+:all      : all
        #    e+:good     : electron ID > 0.7,Originating from IP.
        #    e+:highE    : E_lab> 1.5GeV                                       -> nhighEel
        #    e+:tagged   : E >1.5 GeV and highest E                      -> nTagged
        #       nhighEel == 1: No. of high energy electron should be one.
        # ---
        # e+:good
        ma.cutAndCopyList('e+:good', 'e+:all',
                          Electron_IPcut + ' and '
                          + Pt_cut + ' and '
                          + ElectronIDcut, path=path)
        va.addAlias('ngoodelectron', 'nParticlesInList(e+:good)')
        ma.rankByHighest('e+:good', "p", path=path)
        va.addAlias('egood_pRank', 'extraInfo(p_rank)')

        # --e+:highE
        ma.cutAndCopyList('e+:highE', 'e+:good', ElectronEcut, path=path)
        va.addAlias('nhighEel', 'nParticlesInList(e+:highE)')

        # -e+:tagged
        ma.cutAndCopyList('e+:tagged', 'e+:good', ElectronEcut +
                          ' and egood_pRank==1', path=path)
        va.addAlias('nTagged', 'nParticlesInList(e+:tagged)')

        # --
        # Selection of charged pions
        # -
        #    pi+:all      : all
        #    pi+:good     : Originating from IP. abs(dz)<2.0cm  dr <0.5 cm,
        #                           pt>0.15
        #                           not identified as an electron, (electron ID < 0.7),  -> npion
        # --
        #  pi+:good, npion
        ma.cutAndCopyList('pi+:good', 'pi+:all',
                          Pion_IPcut + ' and '
                          + Pt_cut + ' and '
                          + PionIDcut, path=path)
        va.addAlias('npion', 'nParticlesInList(pi+:good)')
        # ---
        #  Selection of gammas
        #      gamma:all    : all
        #      gamma:good   : E>0.1 GeV     -> nphoton
        # ---
        #
        ma.rankByHighest('gamma:all', "clusterE", path=path)
        va.addAlias('clusterERank', 'extraInfo(clusterE_rank)')
        ma.cutAndCopyList('gamma:good', 'gamma:all', good_cluster, path=path)
        va.addAlias('nphoton', 'nParticlesInList(gamma:good)')

        # --
        #        pi0  reconstruction
        #
        #     pi0:loose        : 0.08 < <Mgg  < 0.17
        #     pi0:highE        : E_pi0  > 0.5 GeV
        # --
        ma.cutAndCopyList('gamma:first', 'gamma:good', good_cluster, path=path)
        ma.cutAndCopyList('gamma:second', 'gamma:good', good_cluster,
                          path=path)
        #
        ma.reconstructDecay('pi0:loose -> gamma:first gamma:second',
                            pi0_mass_wide, dmID=1, path=path)
        ma.rankByHighest('pi0:loose', "p", path=path)
        va.addAlias('pi0_p_Rank', 'extraInfo(p_rank)')
        va.addAlias('npi0loose', 'nParticlesInList(pi0:loose)')
        # - pi0:highE
        ma.cutAndCopyList('pi0:highE', 'pi0:loose', pi0_energy, path=path)
        va.addAlias('npi0highE', 'nParticlesInList(pi0:highE)')

        # --
        #      eta  reconstruction                                                       mode
        #       eta:gg                       :eta->gg                                     2
        #       eta:pipipi0                  :eta ->pipipi0                               3
        #       eta:pipig                    :eta->pipi gamma                             4
        # --
        # mode = 2
        # --
        ma.reconstructDecay('eta:gg -> gamma:first gamma:second',
                            eta_mass_wide, dmID=2, path=path)
        va.addAlias('nmode2', 'nParticlesInList(eta:gg)')
        # ---
        # mode = 3
        #  --
        ma.reconstructDecay('eta:pipipi0 -> pi+:good pi-:good pi0:loose',
                            eta_mass_wide, dmID=3, path=path)
        va.addAlias('nmode3', 'nParticlesInList(eta:pipipi0)')
        # --
        #  mode = 4
        # --
        ma.reconstructDecay('eta:pipig -> pi+:good pi-:good gamma:good',
                            eta_mass_wide, dmID=4, path=path)
        va.addAlias('nmode4', 'nParticlesInList(eta:pipig)')
        # --
        # eta'  reconstruction
        #       eta':pipieta_gg        :eta' -> pipieta (eta->  gg)                      5
        #       eta':pipig             :eta' -> pipi gamma                               6
        # --
        #   mode = 5
        # --
        ma.reconstructDecay("eta':pipieta_gg -> pi+:good pi-:good eta:gg",
                            etap_mass_wide, dmID=5, path=path)
        va.addAlias('nmode5', "nParticlesInList(eta':pipieta_gg)")

        # --
        #  mode=6
        # --
        ma.reconstructDecay("eta':pipig -> pi+:good pi-:good gamma:good",
                            etap_mass_wide, dmID=6, path=path)
        va.addAlias('nmode6', "nParticlesInList(eta':pipig)")
        # --
        # Final skim condition
        # --
        #  Require one tagged electron  and <=2 of other charged tracks.
        #  --
        presel = ' nhighEel == 1 and npion <= 2 '
        va.addAlias('mode_sum',
                    'formula(npi0highE + nmode2 + nmode3 + nmode4 + nmode5 + nmode6 )')

        eventcuts = presel + ' and mode_sum >= 1'
        #
        #  Although a condition of "mode_sum >= 1" looks like very loose,
        #  the reduction rate of this SingleTagPseudoScalar skim is very large, i.e. 1/50,
        #  since the requirements, one high-energy electron and <=2 other charged
        #  tracks, are quite stringent.
        #
        path = self.skim_event_cuts(eventcuts, path=path)
        #
        # Only tagged electron information is added to the standard udst output.
        self.SkimLists = ['e+:highE']
