#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""(Semi-)Leptonic Working Group Skim list building functions for semi-leptonic analyses.
"""

import basf2 as b2
import modularAnalysis as ma
from skim.standardlists.charm import (loadKForBtoHadrons, loadPiForBtoHadrons,
                                      loadStdD0, loadStdDplus, loadStdDstar0,
                                      loadStdDstarPlus, loadPiSkimHighEff,
                                      loadKSkimHighEff, loadSlowPi,
                                      loadSkimHighEffD0_Kpi, loadSkimHighEffDstarPlus_D0pi_Kpi,
                                      loadSkimHighEffD0_Kpipipi, loadSkimHighEffDstarPlus_D0pi_Kpipipi,
                                      loadStdD0_eff20_Kpipi0, loadStdDstarPlus_D0pi_Kpipi0_eff20)
from skim.standardlists.lightmesons import loadStdPi0ForBToHadrons
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from variables import variables as vm
# from ROOT import Belle2

__liaison__ = "Cameron Harris <cameron.harris@adelaide.edu.au>, Tommy Martinov <tommy.martinov@desy.de>"
_VALIDATION_SAMPLE = "mdst16.root"


@fancy_skim_header
class PRsemileptonicUntagged(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^0 \\to \\pi^- e^+`
    * :math:`B^0 \\to \\pi^- \\mu^+`

    Event-level cuts:

    * :math:`\\text{foxWolframR2} < 0.5` constructed using tracks with
      :math:`p_T>0.1\\,\\text{GeV}` and clusters with :math:`E>0.1\\,\\text{GeV}`.
    * :math:`n_{\\text{tracks}} > 4`

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.5`
    * :math:`p > 1.5\\,\\text{GeV}` in CMS frame

    Cuts on muons:

    * :math:`\\text{muonID} > 0.5`
    * :math:`p > 1.5\\,\\text{GeV}` in CMS frame

    Cuts on pions:

    * :math:`\\text{pionID}>0.5`
    * :math:`\\text{muonID}<0.2`
    * :math:`0.060\\,\\text{GeV}<p<0.220\\,\\text{GeV}` in CMS frame

    Cuts on partially reconstructed :math:`B` mesons:

    * :math:`\\cos\\theta_{\\ell,\\,\\pi}<0` in CMS frame.
    """

    __authors__ = ["Lucien Cremaldi", "Racha Cheaib", "Romulus Godang"]
    __description__ = "Skim for partial reconstruction analysis in leptonic group."
    __contact__ = __liaison__
    __category__ = "physics, semileptonic"

    ApplyHLTHadronCut = False

    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)

    def build_lists(self, path):
        ma.cutAndCopyList("pi+:PRSL_eventshape", "pi+:all",
                          cut="pt> 0.1", path=path)
        ma.fillParticleList(decayString="gamma:PRSL_eventshape",
                            cut="E > 0.1", path=path)

        ma.buildEventShape(inputListNames=["pi+:PRSL_eventshape", "gamma:PRSL_eventshape"],
                           allMoments=False,
                           foxWolfram=True,
                           harmonicMoments=False,
                           cleoCones=False,
                           thrust=False,
                           collisionAxis=False,
                           jets=False,
                           sphericity=False,
                           checkForDuplicates=False,
                           path=path)

        path = self.skim_event_cuts("foxWolframR2<0.5 and nTracks>4", path=path)

        ma.cutAndCopyList("e+:PRSemileptonic_1", "e+:all",
                          "useCMSFrame(p) > 1.50 and electronID > 0.5", path=path)
        ma.cutAndCopyList("mu+:PRSemileptonic_1", "mu+:all",
                          "useCMSFrame(p) > 1.50 and muonID > 0.5", path=path)
        ma.cutAndCopyList("pi-:PRSemileptonic_1", "pi-:all",
                          "pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.220", path=path)

        ma.cutAndCopyList("e+:PRSemileptonic_2", "e+:all",
                          "0.600 < useCMSFrame(p) <= 1.50 and electronID > 0.5", path=path)
        ma.cutAndCopyList("mu+:PRSemileptonic_2", "mu+:all",
                          "0.350 < useCMSFrame(p) <= 1.50 and muonID > 0.5", path=path)
        ma.cutAndCopyList("pi-:PRSemileptonic_2", "pi-:all",
                          "pionID>0.5 and muonID<0.2 and 0.060<useCMSFrame(p)<0.160", path=path)

        ma.reconstructDecay("B0:PRSemileptonic_1 ->  pi-:PRSemileptonic_1 e+:PRSemileptonic_1",
                            "useCMSFrame(cos(daughterAngle(0,1)))<0.00", 1, path=path)
        ma.reconstructDecay("B0:PRSemileptonic_2 ->  pi-:PRSemileptonic_1 mu+:PRSemileptonic_1",
                            "useCMSFrame(cos(daughterAngle(0,1)))<0.00", 2, path=path)
        ma.reconstructDecay("B0:PRSemileptonic_3 ->  pi-:PRSemileptonic_2 e+:PRSemileptonic_2",
                            "useCMSFrame(cos(daughterAngle(0,1)))<1.00", 3, path=path)
        ma.reconstructDecay("B0:PRSemileptonic_4 ->  pi-:PRSemileptonic_2 mu+:PRSemileptonic_2",
                            "useCMSFrame(cos(daughterAngle(0,1)))<1.00", 4, path=path)

        return ["B0:PRSemileptonic_1", "B0:PRSemileptonic_2"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.copyLists('B0:all', self.SkimLists, path=path)

        ma.buildRestOfEvent('B0:all', path=path)
        ma.appendROEMask('B0:all', 'basic',
                         'pt>0.05 and dr<2 and -4.0<dz<4.0',
                         'E>0.05',
                         path=path)

        vm.addAlias('d0_p', 'daughter(0, p)')
        vm.addAlias('d1_p', 'daughter(1, p)')
        vm.addAlias('MissM2', 'weMissM2(basic,0)')

        histogramFilename = f'{self}_Validation.root'

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B0:all',
            variables_1d=[
                ('Mbc', 100, 4.0, 5.3, 'Mbc', __liaison__, '', ''),
                ('d0_p', 100, 0, 0.3, 'Signal-side pion momentum', __liaison__, '', ''),
                ('d1_p', 100, 1, 4, 'Signal-side lepton momentum', __liaison__, '', ''),
                ('MissM2', 100, -5, 5, 'Missing mass squared', __liaison__, '', '')
            ],
            variables_2d=[('deltaE', 100, -5, 5, 'Mbc', 100, 4.0, 5.3, 'Mbc vs deltaE', __liaison__, '', '')],
            path=path)


@fancy_skim_header
class SLUntagged(BaseSkim):
    """
    Cuts applied:

    * :math:`p_{\\ell} > 0.35\\,\\text{GeV}`
    * :math:`5.24 < M_{\\text{bc}} < 5.29`
    * :math:`|\\Delta E | < 0.5`
    * :math:`n_{\\text{tracks}} > 4`

    Reconstructed decays:

    * :math:`B^+ \\to \\overline{D}^{0} e^+`
    * :math:`B^+ \\to \\overline{D}^{0} \\mu^+`
    * :math:`B^+ \\to \\overline{D}^{*0} e^+`
    * :math:`B^+ \\to \\overline{D}^{*0} \\mu^+`
    * :math:`B^0 \\to  D^{-} e^+`
    * :math:`B^0 \\to  D^{-} \\mu^+`
    * :math:`B^0 \\to  D^{*-} e^+`
    * :math:`B^0 \\to  D^{*-} \\mu^+`
    """

    __authors__ = ["Phillip Urquijo", "Racha Cheaib"]
    __description__ = (
        "Skim for semileptonic decays, :math:`B` decays "
        "(:math:`B \\to D \\ell\\nu`, where :math:`\\ell=e,\\mu`)"
    )
    __contact__ = __liaison__
    __category__ = "physics, semileptonic"

    ApplyHLTHadronCut = False

    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPi("loose", path=path)
        stdPhotons("loose", path=path)
        stdPi0s("eff40_May2020", path=path)
        stdKshorts(path=path)
        loadStdPi0ForBToHadrons(path=path)
        loadPiForBtoHadrons(path=path)
        loadKForBtoHadrons(path=path)
        loadStdD0(path=path)
        loadStdDstar0(path=path)
        loadStdDplus(path=path)
        loadStdDstarPlus(path=path)

    def build_lists(self, path):
        path = self.skim_event_cuts("nTracks > 4", path=path)
        ma.cutAndCopyList("e+:SLUntagged", "e+:all", "p>0.35", True, path=path)
        ma.cutAndCopyList("mu+:SLUntagged", "mu+:all", "p>0.35", True, path=path)
        Bcuts = "5.24 < Mbc < 5.29 and abs(deltaE) < 0.5"

        BplusChannels = ["anti-D0:all e+:SLUntagged",
                         "anti-D0:all mu+:SLUntagged",
                         "anti-D*0:all e+:SLUntagged",
                         "anti-D*0:all mu+:SLUntagged"
                         ]

        B0Channels = ["D-:all e+:SLUntagged",
                      "D-:all mu+:SLUntagged",
                      "D*-:all e+:SLUntagged",
                      "D*-:all mu+:SLUntagged"
                      ]

        bplusList = []
        for chID, channel in enumerate(BplusChannels):
            ma.reconstructDecay(f"B+:SLUntagged_{chID} -> {channel}", Bcuts, chID, path=path)
            bplusList.append(f"B+:SLUntagged_{chID}")

        b0List = []
        for chID, channel in enumerate(B0Channels):
            ma.reconstructDecay(f"B0:SLUntagged_{chID} -> {channel}", Bcuts, chID, path=path)
            b0List.append(f"B0:SLUntagged_{chID}")

        return b0List + bplusList

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.copyLists('B+:all', [lst for lst in self.SkimLists if "B+" in lst], path=path)

        ma.buildRestOfEvent('B+:all', path=path)
        ma.appendROEMask('B+:all', 'basic',
                         'pt>0.05 and -2<dr<2 and -4.0<dz<4.0',
                         'E>0.05',
                         path=path)

        vm.addAlias('d1_p', 'daughter(1,p)')
        vm.addAlias('MissM2', 'weMissM2(basic,0)')

        histogramFilename = f'{self}_Validation.root'

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B+:all',
            variables_1d=[
                ('cosThetaBetweenParticleAndNominalB', 100, -3.0, 3.0, 'cosThetaBY', __liaison__, '', ''),
                ('Mbc', 100, 5.24, 5.3, 'Mbc', __liaison__, '', ''),
                ('d1_p', 100, 1, 4, 'Signal-side lepton momentum', __liaison__, '', ''),
                ('MissM2', 100, -5, 5, 'Missing mass squared', __liaison__, '', '')
            ],
            variables_2d=[('deltaE', 100, -1, 1, 'Mbc', 100, 5.2, 5.3, 'Mbc vs deltaE', __liaison__, '', '')],
            path=path)


@fancy_skim_header
class B0toDstarl_Kpi_Kpipi0_Kpipipi(BaseSkim):
    """
    Cuts applied:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND dr < 2 AND abs(dz) < 5 AND PID>=0.01``
    * ``slowPi tracks thetaInCDCAcceptance AND dr < 2 AND abs(dz) < 5 AND useCMSFrame(p) < 0.4``
    * :math:`2.5 > p_{\\ell} > 1.1\\,\\text{GeV}`
    * ``lepton with abs(dr) < 0.5 AND abs(dz) < 2 AND thetaInCDCAcceptance AND ID >= 0.95 AND 1.1 < useCMSFrame(p) < 2.5``
    * ``1.8 < M_D0 < 2.0``
    * ``DM_Dstar_D < 0.16``

    Reconstructed decays:

    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^-) e^+`,
    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^- \\pi^0) e^+`,
    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^- \\pi^- \\pi^+) e^+`,
    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^-) mu^+`,
    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^- \\pi^0) mu^+`,
    * :math:`B^{0}\\to D^{*-} (D^{0} \\to K^+ \\pi^- \\pi^- \\pi^+) mu^+`,

    Note:

        This skim uses `skim.standardlists.charm.loadSkimHighEffD0_Kpi`,
        `skim.standardlists.charm.loadSkimHighEffD0_Kpipipi` and
        `skim.standardlists.charm.loadStdD0_eff20_Kpipi0`, where :math:`D^0`
        channel is defined.
        `skim.standardlists.charm.loadSkimHighEffDstarPlus_D0pi_Kpi`,
        `skim.standardlists.charm.loadSkimHighEffDstarPlus_D0pi_Kpipipi`,
        `skim.standardlists.charm.loadStdDstarPlus_D0pi_Kpipi0_eff20`,where the
        :math:`D^{*-}` channel is defined.

        The pion and kaon lists used to define :math:`D^0` and :math:`D^{*-}` are:
        `skim.standardlists.charm.loadPiSkimHighEff`, `skim.standardlists.charm.loadKSkimHighEff` and
        `skim.standardlists.charm.loadSlowPi`
    """
    __authors__ = ["Bae Hanwook, Chiara La Licata"]
    __description__ = ""
    __contact__ = __liaison__
    __category__ = "physics, semileptonic"

    ApplyHLTHadronCut = True
    produce_on_tau_samples = False  # retention is very close to zero on taupair

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdK("all", path=path)
        stdPi0s("eff20_May2020", path=path)
        loadPiSkimHighEff(path=path)
        loadKSkimHighEff(path=path)
        loadSlowPi(path=path)
        loadSkimHighEffD0_Kpi(path=path)
        loadSkimHighEffDstarPlus_D0pi_Kpi(path=path)
        loadSkimHighEffD0_Kpipipi(path=path)
        loadSkimHighEffDstarPlus_D0pi_Kpipipi(path=path)
        loadStdD0_eff20_Kpipi0(path=path)
        loadStdDstarPlus_D0pi_Kpipi0_eff20(path=path)

    def build_lists(self, path):

        ma.cutAndCopyList(
            'e+:sig',
            'e+:all',
            'abs(dr) < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and electronID >= 0.95 and 1.1 < useCMSFrame(p) < 2.5 ',
            path=path)
        ma.cutAndCopyList(
            'mu+:sig',
            'mu+:all',
            'abs(dr) < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and muonID >= 0.95 and 1.1 < useCMSFrame(p) < 2.5',
            path=path)

        B0_channels = ["D*-:D0_Kpi_skimhigheff e+:sig",
                       "D*-:D0_Kpipi0_eff20 e+:sig",
                       "D*-:D0_Kpipipi_skimhigheff e+:sig",
                       "D*-:D0_Kpi_skimhigheff mu+:sig",
                       "D*-:D0_Kpipi0_eff20 mu+:sig",
                       "D*-:D0_Kpipipi_skimhigheff mu+:sig"]

        B0_list = []

        for chID, channel in enumerate(B0_channels):
            ma.reconstructDecay("B0:Dstl_kpi_kpipi0_kpipipi" + str(chID) + " -> " + channel, "", chID, path=path)
            B0_list.append("B0:Dstl_kpi_kpipi0_kpipipi" + str(chID))

        return B0_list

# new SLskim


@fancy_skim_header
class BtoDl_and_ROE_e_or_mu_or_lowmult(BaseSkim):
    """
    Reconstructed decay modes:

    * :math:`B^- \\to D^0 e^-`,  :math:`B^- \\to D^0 \\mu^-`
    * anti-:math:`B^0 \\to D^+ e^-`,  anti-:math:`B^0 \\to D^+ \\mu^-`
    * anti-:math:`B^0 \\to D^{*+}(D^0 \\pi^+) e^-`,  anti-:math:`B^0 \\to D^{*+}(D^0 \\pi^+) \\mu^-`

    Additional ROE cuts:

    * The logical OR of the following:

      * identified :math:`e^{\\pm}` with :math:`p(CM) < 3.0` GeV
      * identified :math:`\\mu^{\\pm}` with :math:`p(CM) < 3.0` GeV
      * identified :math:`\\gamma` with :math:`E(CM) > 1.4` GeV
      * Two or fewer charged tracks
      * :math:`E_{ECL} < 2.0` GeV

    Cuts on electrons:

    * :math:`\\text{electronID} > 0.3`
    * :math:`p_t > 0.3\\,\\text{GeV}` in lab frame,  :math:`p > 0.5\\,\\text{GeV}` in lab frame
    * :math:`dr < 0.5`,  :math:`|dz| < 2`
    * :math:`\\text{thetaInCDCAcceptance}`

    Cuts on muons:

    * :math:`\\text{muonID} > 0.9`
    * :math:`p_t > 0.4\\,\\text{GeV}` in lab frame,  :math:`p > 0.7\\,\\text{GeV}` in lab frame
    * :math:`dr < 0.5`,  :math:`|dz| < 2`

    Charged mask for ROE:

    * :math:`p_t>0.05\\,\\text{GeV}`
    * :math:`dr < 5\\,\\text{cm}`,  :math:`|dz| < 10\\,\\text{cm}`

    ECL cluster mask for ROE:

    * :math:`\\text{clusterNHits}>1.5`,  :math:`\\theta` in CDC acceptance
    * :math:`E>0.080,\\text{GeV}`
    """

    __authors__ = ["Bob Kowalewski"]
    __description__ = "Skim for semileptonic tags with an ROE electron, muon, photon or a low-multiplicity signal decay"
    __contact__ = __liaison__
    __category__ = "physics, semileptonic"

    ApplyHLTHadronCut = True

    TestSampleProcess = "charged"

    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdKshorts(path=path)

    def build_lists(self, path):

        eIDCut = "electronID > 0.3"
        muIDCut = "muonID > 0.9"
        ePCut = "p > 0.5"
        ePtCut = "pt > 0.3"
        muPCut = "p > 0.7"
        muPtCut = "pt > 0.4"
        lepTrkCuts = "dr < 0.5 and abs(dz) < 2"
        hadTrkCuts = "dr < 2.0 and abs(dz) < 4"
        gammaCuts = "[clusterNHits>1.5] and thetaInCDCAcceptance"
        gammaECuts = "[[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.025] or [clusterReg==3 and E>0.040]]"
        cleanMask = ('cleanMask', 'pt>0.05 and dr < 5 and abs(dz) < 10',
                     f'{gammaCuts} and E>0.080 and minC2TDist>20.0 and abs(clusterTiming)<200')
        gammaSignalECut = "1.4"
        BSLRecoCut = "InvM > 2.5 and cosBY > -3.0 and cosBY < 1.5"
        BSLSkimCut = f"e_ROE_pCM < 3.0 or mu_ROE_pCM < 3.0 or gamma_ROE_ECM > {gammaSignalECut}" +\
                     " or nROE_Ch < 2.5 or E_extra_ROE < 1.6"

        vm.addAlias('pCM', 'useCMSFrame(p)')
        vm.addAlias('ECM', 'useCMSFrame(E)')
        vm.addAlias('cosBY', 'cosThetaBetweenParticleAndNominalB')
        # electrons and muons
        ma.fillParticleList("e-:sig", f"{lepTrkCuts} and thetaInCDCAcceptance and {ePtCut} and {ePCut}", path=path)
        ma.applyCuts('e-:sig', eIDCut, path=path)

        ma.fillParticleList("mu-:sig", f"{muIDCut} and {lepTrkCuts} and {muPtCut} and {muPCut}", path=path)
        total_leptons = "formula( nParticlesInList(e-:sig) + nParticlesInList(mu-:sig) )"
        # don't waste time if there are no leptons
        path = self.skim_event_cuts(f"{total_leptons} > 0", path=path)

        # charged K and pi candidates
        ma.fillParticleList("K-:BtoDl", f"binaryPID(321,211) > 0.1 and {hadTrkCuts} and thetaInCDCAcceptance and pt>0.1", path=path)
        ma.fillParticleList(
            "pi+:BtoDl",
            f"binaryPID(211,321) > 0.1 and {hadTrkCuts} and thetaInCDCAcceptance and pt>0.1",
            path=path)
        ma.fillParticleList("pi+:slow", f"{hadTrkCuts} and thetaInCDCAcceptance and p>0.05", path=path)
        # gamma and pi0 candidates
        ma.fillParticleList("gamma:eff50", f'{gammaCuts} and {gammaECuts}', path=path)
        ma.reconstructDecay("pi0:eff50 -> gamma:eff50 gamma:eff50", '0.114<InvM<0.150', path=path)
        ma.cutAndCopyList("gamma:e90", "gamma:eff50", "E>0.090", path=path)
        ma.reconstructDecay("pi0:e90 -> gamma:e90 gamma:e90", '0.118<InvM<0.148', path=path)
        ma.cutAndCopyList('K_S0:good', 'K_S0:merged', cut='significanceOfDistance>2.5', path=path)

        # D reconstruction - only clean modes
        ma.reconstructDecay("D0:Kpi_BtoDl -> K-:BtoDl pi+:BtoDl", cut="abs(dM) < 0.025", dmID=1, path=path)
        ma.rankByLowest("D0:Kpi_BtoDl", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D0:KK        -> K-:BtoDl K+:BtoDl", cut="abs(dM) < 0.020", dmID=2, path=path)
        ma.rankByLowest("D0:KK", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D0:K3pi      -> K-:BtoDl pi+:BtoDl pi-:BtoDl pi+:BtoDl", cut="abs(dM) < 0.020", dmID=3, path=path)
        ma.rankByLowest("D0:K3pi", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D0:Kpi0pi    -> K-:BtoDl pi0:e90 pi+:BtoDl", cut="abs(dM)-0.005 < 0.050", dmID=4, path=path)
        ma.rankByLowest("D0:Kpi0pi", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D0:piKspi      -> pi+:BtoDl K_S0:good pi-:BtoDl", cut="abs(dM) < 0.020", dmID=5, path=path)
        ma.rankByLowest("D0:piKspi", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D0:Kspi0_BtoDl -> K_S0:good pi0:e90", cut="abs(dM)-0.005 < 0.050", dmID=6, path=path)
        ma.rankByLowest("D0:Kspi0_BtoDl", variable="abs(dM)", numBest=15, path=path)
        ma.copyLists("D0:sig", ['D0:Kpi_BtoDl', 'D0:KK', 'D0:K3pi', 'D0:Kpi0pi', 'D0:piKspi', 'D0:Kspi0_BtoDl'], path=path)

        ma.reconstructDecay("D+:Kpipi_BtoDl -> K-:BtoDl pi+:BtoDl pi+:BtoDl", cut="abs(dM) < 0.020", dmID=1, path=path)
        ma.rankByLowest("D+:Kpipi_BtoDl", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D+:Kspi_BtoDl -> K_S0:good pi+:BtoDl", cut="abs(dM) < 0.020", dmID=2, path=path)
        ma.rankByLowest("D+:Kspi_BtoDl", variable="abs(dM)", numBest=15, path=path)
        ma.reconstructDecay("D+:KKpi     -> K-:BtoDl K+:BtoDl pi+:BtoDl", cut="abs(dM) < 0.015", dmID=3, path=path)
        ma.rankByLowest("D+:KKpi", variable="abs(dM)", numBest=15, path=path)
        ma.copyLists("D+:sig", ['D+:Kpipi_BtoDl', 'D+:Kspi_BtoDl', 'D+:KKpi'], path=path)

        # No explicit reconstruction of D*0 to D0 pi0/gamma or D*+ to D0 pi+ (included in feeddown)

        ma.reconstructDecay("D*+:D0        -> pi+:slow D0:sig", cut="abs(dQ) < 0.0035", dmID=11, path=path)
        ma.rankByLowest("D*+:D0", variable="abs(dQ)", numBest=15, path=path)
        ma.copyLists("D*+:sig", ['D*+:D0'], path=path)

        # B reconstruction.  Use wide cut on cosThetaBY.

        ma.reconstructDecay('anti-B0:Dpe  ->  e-:sig D+:sig ?nu ', BSLRecoCut, dmID=200, path=path)
        ma.reconstructDecay('anti-B0:Dpmu -> mu-:sig D+:sig ?nu ', BSLRecoCut, dmID=201, path=path)
        ma.copyLists('anti-B0:Dpl', ['anti-B0:Dpe', 'anti-B0:Dpmu'], path=path)
        ma.rankByHighest('anti-B0:Dpl', variable='InvM', numBest=9, path=path)

        ma.reconstructDecay('anti-B0:Dspe  ->  e-:sig D*+:sig ?nu ', BSLRecoCut, dmID=100, path=path)
        ma.reconstructDecay('anti-B0:Dspmu -> mu-:sig D*+:sig ?nu ', BSLRecoCut, dmID=101, path=path)
        ma.copyLists('anti-B0:Dspl', ['anti-B0:Dspe', 'anti-B0:Dspmu'], path=path)
        ma.rankByHighest('anti-B0:Dspl', variable='InvM', numBest=9, path=path)

        ma.reconstructDecay('B-:D0e  ->  e-:sig D0:sig     ?nu ', BSLRecoCut, dmID=400, path=path)
        ma.reconstructDecay('B-:D0mu -> mu-:sig D0:sig     ?nu ', BSLRecoCut, dmID=401, path=path)
        ma.copyLists('B-:D0l', ['B-:D0e', 'B-:D0mu'], path=path)
        ma.rankByHighest('B-:D0l', variable='InvM', numBest=9, path=path)

        # main SL B lists
        ma.copyLists('B-:SL', ['B-:D0l'], path=path)
        ma.copyLists('anti-B0:SL', ['anti-B0:Dspl', 'anti-B0:Dpl'], path=path)

        # Build ROE and look for e-, mu-, charged multiplicity and E_ECL

        for Btype in ['B-:SL', 'anti-B0:SL']:

            roe_path = b2.create_path()
            deadEndPath = b2.create_path()
            # Execute the filter module:
            ma.buildRestOfEvent(Btype, fillWithMostLikely=True, path=path)
            ma.appendROEMasks(Btype, [cleanMask], path=path)
            ma.signalSideParticleFilter(Btype, '', roe_path, deadEndPath)

            ma.fillParticleList(
                "e-:roeB",
                f"isInRestOfEvent == 1 and {eIDCut} and {lepTrkCuts} and thetaInCDCAcceptance and {ePtCut} and {ePCut}",
                path=roe_path)
            ma.fillParticleList(
                "mu-:roeB",
                f"isInRestOfEvent == 1 and {muIDCut} and {lepTrkCuts} and {muPtCut} and {muPCut}",
                path=roe_path)
            ma.fillParticleList("gamma:roeB", f'{gammaCuts} and ECM > {gammaSignalECut}', path=roe_path)
            ma.rankByHighest('e-:roeB', 'pCM', numBest=1, path=roe_path)
            ma.rankByHighest('mu-:roeB', 'pCM', numBest=1, path=roe_path)
            ma.rankByHighest('gamma:roeB', 'ECM', numBest=1, path=roe_path)
            ma.variableToSignalSideExtraInfo('e-:roeB', {'pCM': 'e_ROEB_pCM'}, path=roe_path)
            ma.variableToSignalSideExtraInfo('mu-:roeB', {'pCM': 'mu_ROEB_pCM'}, path=roe_path)
            ma.variableToSignalSideExtraInfo('gamma:roeB', {'ECM': 'gamma_ROEB_ECM'}, path=roe_path)

            path.for_each('RestOfEvent', 'RestOfEvents', path=roe_path)

        vm.addAlias('e_ROE_pCM', 'extraInfo(e_ROEB_pCM)')
        vm.addAlias('mu_ROE_pCM', 'extraInfo(mu_ROEB_pCM)')
        vm.addAlias('gamma_ROE_ECM', 'extraInfo(gamma_ROEB_ECM)')
        vm.addAlias('nROE_Ch', 'nROE_Charged(cleanMask)')
        vm.addAlias('E_extra_ROE', 'useCMSFrame(roeEextra(cleanMask))')

        # Only keep events whose ROE has an extra e or mu, or Ntrk<3, or E_ECL<2.0 GeV

        ma.applyCuts('B-:SL', f'passesCut({BSLSkimCut})', path=path)
        ma.applyCuts('anti-B0:SL', f'passesCut({BSLSkimCut})', path=path)
        b_sl_list = ['B-:SL', 'anti-B0:SL']

        return b_sl_list

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        vm.addAlias('d0_p', 'daughter(0, p)')
        vm.addAlias('d1_p', 'daughter(1, p)')

        histogramFilename = f'{self}_Validation.root'

        create_validation_histograms(
            rootfile=histogramFilename,
            particlelist='B-:SL',
            variables_1d=[
                ('cosBY', 90, -3.0, 1.5, 'cosThetaBY', __liaison__,
                 'Cosine of angle between the reconstructed B and the nominal B', '',
                 'cos#theta_{BY}', 'Candidates'),
                ('InvM', 100, 2.5, 5.3, 'M(D(*)+l)', __liaison__,
                 'Invariant mass distribution of D(*)l system', '', 'm(D(*)l) [GeV]', 'Candidates / 28 MeV'),
                ('useCMSFrame(d0_p)', 100, 0, 3.0, 'Tag lepton momentum in CMS', __liaison__,
                 'Momentum of tag-side lepton in CMS', '', 'p(l) [GeV]', 'Candidates / 30 MeV'),
                ('useCMSFrame(d1_p)', 100, 0, 3.0, 'Tag D(*) momentum in CMS', __liaison__,
                 'Momentum of tag-side D(*) meson in CMS', '', 'p(D(*)) [GeV]', 'Candidates / 30 MeV'),
                ('e_ROE_pCM', 100, 0, 3.0, 'ROE e momentum in CMS', __liaison__,
                 'Momentum of highest-momentum ROE electron in CMS', '', 'p(e) [GeV]', 'Candidates / 30 MeV'),
                ('mu_ROE_pCM', 100, 0, 3.0, 'ROE mu momentum in CMS', __liaison__,
                 'Momentum of highest-momentum ROE muon in CMS', '', 'p($\\mu$) [GeV]', 'Candidates / 30 MeV'),
                ('gamma_ROE_ECM', 100, 0, 5.0, 'ROE gamma energy in CMS', __liaison__,
                 'Energy of most-energetic ROE photon', '', 'E($\\gamma$) [GeV]', 'Candidates / 50 MeV'),
                ('nROE_Ch', 20, 0, 20.0, 'N(trk) in ROE', __liaison__,
                 'Number of tracks in ROE', '', 'tracks', 'Candidates'),
                ('E_extra_ROE', 100, 0, 2.5, 'E_ECL (CMS) in ROE', __liaison__,
                 'Extra energy in ROE', '', 'E_{extra} [GeV]', 'Candidates / 25 MeV'),
            ],
            path=path)
