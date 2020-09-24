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
from modularAnalysis import reconstructDecay
from modularAnalysis import rankByHighest


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


@fancy_skim_header
class SingleTagPseudoScalar(BaseSkim):
    """
    **Physics channel**: ;math: `e^{+}e^{-} \\to  e^{\\pm} (e^{\\mp}) \\pi^{0}/\\eta/\\eta^{\prime}/\\pi^{0} \\pi^{0}`

    Skim list contains single-tag two-photon meson production for the meson transition
    form factor measurement.
    """
    __authors__ = ["Hisaki Hayashii"]
    __contact__ = "Hisaki Hayashii <hisaki.hayashii@desy.de>"
    __description__ = "Single-tag two-photon  skim list for the meson trangition form-factor (TFF)  measurement."
    __category__ = "physics, low multiplicity"
    """
    **Decay Modes**:

    *1 :math:`\\pi^{0}\\to \\gamma \gamma `,
    *2 :math:`\\eta \\to \gamma\\gamma `,
    *3 :math:`\\eta \\to \\pi^{+}\\pi^{-}\\pi^{0}`,
    *4 :math:`\\eta \\to \\pi^{+}\\pi^{-}\\gamma`,
    *5 :math:`\\eta^{\prime} \\to \\pi^{+}\\pi^{-}\\eta(\\to \gamma\gamma)`,
    *6 :math:`\\eta^{\prime} \\to \\pi^{+}\\pi^{-}\\gamma`,



    **Additional Cuts**:
    *  electron Tag:  electronID> 0.7, E_lab > 1.5 GeV, abs(dz)<2.0 cm, dr <0.5 cm
    *  no. of good pion <= 2,
    *       (condition of good pion; pt>0.15GeV, abs(dz)<2.0 cm, dr<0.5 cm, electronID<0.7)
    *  pi0 condition ``0.03 GeV < M(gg) < 0.17 GeV``
    *                              (For e-pi0 case  E_pi0 > 0.5 GeV)
    *  eta condtion    ``0.50 GeV  < InvM < 0.60 GeV``
    *  eta' condtion    ``0.90 GeV < InvM  < 1.10 GeV``
    """

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def build_lists(self, path):
        # ------------------------------------------------------------
        # Cut conditions
        # --
        # tagged electron
        GoodTrack = 'pt>0.15 and abs(dz)<2.0 and dr<0.5'
        ElectronIDcut = 'electronID > 0.7'
        ElectronEcut = 'E >1.5'
        # --
        # charged pion
        Ptcut = 'pt >0.15'
        PionIDcut = 'electronID < 0.7'
        # --
        # photons
        good_cluster = 'clusterE > 0.1'
        # --
        # pi0 loose mass region.
        pi0_mass_wide = '0.06<InvM<0.17'
        # pi0_energy.  only applied for e(e)pi0 mode,
        pi0_energy = ' E>0.5'
        # --
        # eta, eta' loose mass region
        eta_mass_wide = '0.50< InvM <0.60'
        etap_mass_wide = '0.91< InvM <1.10'
        # ------------------------end of cut parameters ------------------------------
        # ---
        #  Tagged electron/positron.
        # --
        #    e+:all         : all
        #    e+:good     : electron ID > 0.7,Originating from IP.
        #    e+:highE    : E_lab> 1.5GeV                                                           -> nhighEel
        #    e+:tagged  : E >1.5 GeV and highest E                                          -> nTagged
        #       nhighEel == 1    : No of high energy electron should be one.
        #  [Note]
        #      No theta cuts are applied..
        #      No.  brems correction.
        # ---
        # va.addAlias('cms_p','useCMSFrame(p)')
        # va.addAlias('cms_clusterE','useCMSFrame(clusterE)')
        # va.addAlias('cms_E','useCMSFrame(E)')
        # va.addAlias('cms_theta','useCMSFrame(theta)')
        #
        # e+:good
        ma.cutAndCopyList('e+:good', 'e+:all', GoodTrack+' and ' + ElectronIDcut, path=path)
        va.addAlias('ngoodelectron', 'nParticlesInList(e+:good)')
        #
        # ma.printVariableValues('',['ngoodelectron'], path=path)
        # ma.printVariableValues('e+:good',['p', 'pt','E','clusterE', 'theta','phi',
        #                                  'dr', 'dz','charge', 'pionID', 'electronID' ],path=path)
        ma.rankByHighest('e+:good', "p", path=path)
        va.addAlias('egood_pRank', 'extraInfo(p_rank)')
        #  print e+:good-
        # ma.printVariableValues('e+:good',['egood_pRank','p','pt','E','clusterE',
        #                          'theta','phi','dr', 'dz','isFromECL','isFromTrack',
        #                        'cms_p','cms_clusterE','cms_E','cms_theta','electronID',
        #                          'pionID','chiProb','clusterTrackMatch','
        #                         nECLClusterTrackMatches' ]
        #                        ,path=path)
        # --e+:highE
        ma.cutAndCopyList('e+:highE', 'e+:good',  ElectronEcut, path=path)
        va.addAlias('nhighEel', 'nParticlesInList(e+:highE)')
        # ma.printVariableValues('',['nhighEel'], path=path)
        # ma.printVariableValues('e+:highE',['egood_pRank','p','pt','E',
        #                                        'clusterE', 'theta','phi'],path=path)
        # -e+:tagged
        ma.cutAndCopyList('e+:tagged', 'e+:good',  ElectronEcut +
                          ' and egood_pRank==1', path=path)
        va.addAlias('nTagged', 'nParticlesInList(e+:tagged)')
        # ma.printVariableValues('',['nTagged'], path=path)
        # ma.printVariableValues('e+:tagged',['egood_pRank','p','pt',
        #                                 'E','clusterE', 'theta','phi'],path=path)
        # --
        # Selection of charged pion
        #    pi+:all         : all
        #    pi+:good     :  Originating from IP. abs(dz)<2.0cm  dr <0.5 cm,
        #                           pt>0.15
        #                           not identified as an  electron, ( electron ID < 0.7),     -> npion
        #      0<= npion ,<=2
        # ma.printVariableValues('pi+:all',['p', 'pt','E','clusterE','theta',
        #              'phi', 'dr', 'dz','charge', 'pionID', 'electronID' ],path=path)
        #
        # pi+:good, npion
        ma.cutAndCopyList('pi+:good', 'pi+:all',
                          GoodTrack+' and ' + Ptcut + ' and  ' + PionIDcut, path=path)
        va.addAlias('npion', 'nParticlesInList(pi+:good)')
        #
        # ma.printVariableValues('',['npion'], path=path)
        # ma.printVariableValues('pi+:good',['p', 'pt','E','clusterE', 'theta',
        #                             'phi', 'dr', 'dz','charge', 'pionID', 'electronID' ],path=path)
        # ---
        #  gamma selection
        #      gamma:all       :all
        #      gamma:good   : E>0.1 GeV     -> nphoton
        # --- order gamma list according to the energy
        ma.rankByHighest('gamma:all', "clusterE", path=path)
        va.addAlias('clusterERank', 'extraInfo(clusterE_rank)')
        # ma.printVariableValues('gamma:all',['clusterERank', 'p','theta','phi',
        #                          'clusterE','clusterTheta','clusterPhi',
        #                            'clusterTiming','clusterReg'],path=path)
        # -- gamma: good
        #
        ma.cutAndCopyList('gamma:good', 'gamma:all',  good_cluster, path=path)
        # ma.printVariableValues('gamma:good',['clusterERank', 'p',
        #        'theta','phi','clusterE','clusterTheta','clusterPhi',
        #        'clusterTiming','clusterReg'                 ],path=path)
        va.addAlias('nphoton', 'nParticlesInList(gamma:good)')
        #
        #        pi0 /eta reconstruction
        #
        #     pi0:loose         : 0.08 < <Mgg  < 0.17
        #     pi0:highE        :  + E_pi0  > 0.5 GeV
        # not used pi0_polar     =       ' abs(cos(theta))<0.8'
        ma.cutAndCopyList('gamma:first', 'gamma:good',  good_cluster, path=path)
        ma.cutAndCopyList('gamma:second', 'gamma:good', good_cluster,
                          path=path)
        # pi0:loose
        ma.reconstructDecay('pi0:loose -> gamma:first gamma:second',
                            pi0_mass_wide, dmID=1, path=path)
        ma.rankByHighest('pi0:loose', "p", path=path)
        va.addAlias('pi0_p_Rank', 'extraInfo(p_rank)')
        # ma.printVariableValues('pi0:loose',['pi0_p_Rank', 'p', 'pt',
        #             'theta','phi', 'M','charge'], path=path)
        va.addAlias('npi0loose', 'nParticlesInList(pi0:loose)')
        # ma.printVariableValues('',['npi0loose'], path=path)
        # pi0:highE
        ma.cutAndCopyList('pi0:highE', 'pi0:loose',  pi0_energy, path=path)
        va.addAlias('npi0highE', 'nParticlesInList(pi0:highE)')
        # ma.printVariableValues('',['npi0highE'], path=path)
        #
        # eta- resonstruction
        #       eta:gg                       :eta->gg                                             2
        #       eta:pipipi0                :eta ->pipipi0                                     3
        #       eta:pipig                   :eta->pipi gamma                              4
        # eta'  reconstruction
        #       eta':pipieta_gg        :eta' -> pipieta (eta->  gg)                 5
        #       eta':pipig                 :eta' -> pipi gamma                           6
        # --
        #
        # eta:gg  ; mode=2
        #
        ma.reconstructDecay('eta:gg -> gamma:first gamma:second',
                            eta_mass_wide, dmID=2, path=path)
        # ma.printVariableValues('eta:gg',[ 'p', 'pt','theta','phi', 'M','InvM','charge'], path=path)
        va.addAlias('nmode2', 'nParticlesInList(eta:gg)')
        # ma.printVariableValues('',['nmode2'], path=path)
        #
        # eta:pipipi0  ; mode=3
        #
        ma.reconstructDecay('eta:pipipi0 -> pi+:good pi-:good pi0:loose',
                            eta_mass_wide, dmID=3, path=path)
        # ma.printVariableValues('eta:pipipi0',[ 'p', 'pt','theta','phi', 'M','InvM','charge'], path=path)
        va.addAlias('nmode3', 'nParticlesInList(eta:pipipi0)')
        # ma.printVariableValues('',['nmode3'], path=path)
        #
        # eta:pipig  ; mode=4
        #
        ma.reconstructDecay('eta:pipig -> pi+:good pi-:good gamma:good',
                            eta_mass_wide, dmID=4, path=path)
        # ma.printVariableValues('eta:pipig',[ 'p', 'pt','theta','phi', 'M','InvM','charge'], path=path)
        va.addAlias('nmode4', 'nParticlesInList(eta:pipig)')
        # ma.printVariableValues('',['nmode4'], path=path)
        #
        # eta':pipieta_gg  ; mode=5
        #
        ma.reconstructDecay("eta':pipieta_gg -> pi+:good pi-:good eta:gg",
                            etap_mass_wide, dmID=5, path=path)
        # ma.printVariableValues("eta':pipieta_gg",[ 'p', 'pt','theta','phi', 'M','InvM','charge'], path=path)
        va.addAlias('nmode5', "nParticlesInList(eta':pipieta_gg)")
        # ma.printVariableValues('',['nmode5'], path=path)
        #
        # eta':pipig   : mode=6
        #
        ma.reconstructDecay("eta':pipig -> pi+:good pi-:good gamma:good",
                            etap_mass_wide, dmID=6, path=path)
        # ma.printVariableValues("eta':pipig",[ 'p', 'pt','theta','phi', 'M','InvM','charge'], path=path)
        va.addAlias('nmode6', "nParticlesInList(eta':pipig)")
        # ma.printVariableValues('',['nmode6'], path=path)
        #
        #
        # pi0   = '  npi0highE >= 1'
        # eta   = '  nmode2 >= 1 or nmode3 >= 1 or nmode4 >= 1'
        # etap = ' nmode5 >= 1  or  nmode6 >= 1'
        va.addAlias('mode_sum',
                    'formula(npi0highE + nmode2 + nmode3 + nmode4 + nmode5 + nmode6 )')
        #
        # Final skim condition
        #
        presel = ' nhighEel == 1  and  npion <= 2 '
        eventcuts = presel + ' and  mode_sum >= 1 '
        #
        path = self.skim_event_cuts(eventcuts, path=path)
        #
        self.SkimLists = ['e+:highE', 'pi0:highE', 'gamma:good', 'pi+:good']
