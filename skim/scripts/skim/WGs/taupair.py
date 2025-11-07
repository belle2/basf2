#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

""" Skim list building functions for tau-pair analyses """

import modularAnalysis as ma
from skim.standardlists.lightmesons import (loadStdAllF_0, loadStdAllKstar0,
                                            loadStdAllPhi, loadStdAllRho0)
from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from variables import variables as vm
from stdV0s import stdKshorts
import variables.collections as vc
import variables.utils as vu

__liaison__ = "Kenji Inami <kenji.inami@desy.de>"
_VALIDATION_SAMPLE = "mdst16.root"


def tauskim_particle_selection(label, path):
    # Track
    trackCuts = "-3.0 < dz < 3.0 and dr < 1.0"
    ma.cutAndCopyList(f"pi+:{label}", "pi+:all", trackCuts, path=path)

    # pi0

    gammaDetectorLocation = {
        "FWD": "clusterReg == 1",
        "BRL": "clusterReg == 2",
        "BWD": "clusterReg == 3"
    }

    gammaForPi0lists = []
    for g in gammaDetectorLocation.keys():
        gammaForPi0Cuts = gammaDetectorLocation[g]
        gammaForPi0Cuts += ' and abs(clusterTiming) < 200'
        gammaForPi0Cuts += ' and thetaInCDCAcceptance'
        gammaForPi0Cuts += ' and clusterNHits > 1.5'
        gammaForPi0 = f'gamma:looseForPi0{label}{g}'
        gammaForPi0lists.append(gammaForPi0)
        ma.cutAndCopyLists(gammaForPi0, 'gamma:all', gammaForPi0Cuts, path=path)

    # -# -- -- cos of opening angle between the photons
    vm.addAlias(
        'cosAngle2Photons',
        'formula((daughter(0, px) * daughter(1, px) + '
        'daughter(0, py) * daughter(1, py) + '
        'daughter(0, pz) * daughter(1, pz) ) / daughter(0, p) / daughter(1, p) )')

    vm.addAlias('leadingclusterE', 'formula(max(daughter(0, clusterE),daughter(1, clusterE)))')
    vm.addAlias('subleadingclusterE', 'formula(min(daughter(0, clusterE),daughter(1, clusterE)))')

    # Determine pi0 reco for individual Detector Parts
    Pi0CutLabel = ["leadingclusterE", "subleadingclusterE", "cosAngle2Photons", "p"]
    Pi0CutValue = {
        "FWD,FWD": [0.5625,        0.1625,        0.9458,        0.9444],
        "BRL,BRL": [0.4125,        0.0625,        0.8875,        0.6333],
        "BWD,BWD": [0.4125,        0.1125,        0.8708,        0.6111],
        "BRL,FWD": [0.3625,        0.0875,        0.8875,        0.5889],
        "BRL,BWD": [0.3625,        0.0875,        0.8875,        0.5889]
    }

    Pi0lists = []
    for cut in Pi0CutValue.keys():
        gammalists = cut.split(",")
        CurrentPi0List = f'pi0:fromLooseGammas{label}{gammalists[0]}{gammalists[1]}'
        Pi0lists.append(CurrentPi0List)
        Pi0Cut = '0.115 < M < 0.152'
        for i, c in enumerate(Pi0CutLabel):
            Pi0Cut += f' and {c} > {Pi0CutValue[cut][i]}'

        ma.reconstructDecay(f'{CurrentPi0List} -> gamma:looseForPi0{label}{gammalists[0]} gamma:looseForPi0{label}{gammalists[1]}',
                            Pi0Cut, path=path)

    ma.copyLists(f'pi0:{label}', Pi0lists, path=path)
    ma.cutAndCopyList(f'gamma:pi0_{label}', 'gamma:all', f'isDescendantOfList(pi0:{label}) == 1', path=path)

    # gamma
    gammaCuts = 'E > 0.2'
    gammaCuts += ' and abs(clusterTiming) < 200'
    gammaCuts += ' and thetaInCDCAcceptance'
    gammaCuts += ' and clusterNHits > 1.5'
    gammaCuts += f' and isDescendantOfList(pi0:{label}) == 0'
    ma.cutAndCopyList(f'gamma:nonpi0_{label}', 'gamma:all', gammaCuts, path=path)

    ma.copyLists(f'gamma:{label}', [f'gamma:pi0_{label}', f'gamma:nonpi0_{label}'], path=path)


@fancy_skim_header
class TauLFV(BaseSkim):
    """
    **Channel**: :math:`\\tau \\to l \\gamma, lll, l \\pi^0, l \\eta, l \\eta', l K_S, l f_0, l V^0, lhh, llp, phh, \\lambda\\pi`

    **Output particle lists**: ``e+:taulfv, mu+:taulfv, pi+:taulfv, K+:taulfv, p+:taulfv``,
    ``gamma:taulfv, pi0:taulfv, K_S0:taulfv, eta:taulfv, eta':taulfv``,
    ``omega:taulfv, Lambda0:taulfv_p, Lambda0:taulfv_antip``

    **Criteria for 1 prong final states**: Number of good tracks < 5, :math:`1.0 < M < 2.0` GeV, :math:`-1.5 < \\Delta E < 0.5` GeV

    **Criteria for >1 prong final states**: Number of good tracks < 7, :math:`1.4 < M < 2.0` GeV, :math:`-1.0 < \\Delta E < 0.5` GeV
    """
    __authors__ = ["Kenji Inami"]
    __description__ = "Skim for Tau LFV decays."
    __contact__ = __liaison__
    __category__ = "physics, tau"

    ApplyHLTHadronCut = False
    produce_on_tau_samples = False  # retention is too high on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPr("all", path=path)
        stdPhotons("all", path=path)
        loadStdAllRho0(path=path)
        loadStdAllKstar0(path=path)
        loadStdAllPhi(path=path)
        loadStdAllF_0(path=path)

    def build_lists(self, path):
        # particle selection
        trackCuts = "-3.0 < dz < 3.0 and dr < 1.0"
        ma.cutAndCopyList("e+:taulfv", "e+:all", trackCuts, path=path)
        ma.cutAndCopyList("mu+:taulfv", "mu+:all", trackCuts, path=path)
        ma.cutAndCopyList("pi+:taulfv", "pi+:all", trackCuts, path=path)
        ma.cutAndCopyList("K+:taulfv", "K+:all", trackCuts, path=path)
        ma.cutAndCopyList("p+:taulfv", "p+:all", trackCuts, path=path)

        ma.reconstructDecay("K_S0:taulfv -> pi+:all pi-:all", "0.3 < M < 0.7", path=path)

        gammaCuts = "E > 0.20 and clusterNHits > 1.5 and thetaInCDCAcceptance"
        ma.cutAndCopyList("gamma:taulfv", "gamma:all", gammaCuts, path=path)

        gammaLooseCuts = "E > 0.1 and thetaInCDCAcceptance and clusterNHits > 1.5"
        ma.cutAndCopyLists("gamma:taulfvloose", "gamma:all", gammaLooseCuts, path=path)
        # pi0
        ma.reconstructDecay("pi0:taulfv -> gamma:taulfvloose gamma:taulfvloose", "0.115 < M < 0.152", path=path)
        # eta
        ma.reconstructDecay("eta:taulfv1 -> gamma:taulfvloose gamma:taulfvloose", "0.4 < M < 0.6", path=path)
        ma.reconstructDecay("eta:taulfv2 -> pi0:taulfv pi-:all pi+:all", "0.4 < M < 0.6", path=path)
        ma.copyLists("eta:taulfv", ["eta:taulfv1", "eta:taulfv2"], path=path)
        # eta'
        ma.reconstructDecay("eta':taulfv1 -> pi+:all pi-:all gamma:taulfvloose", "0.8 < M < 1.1", path=path)
        ma.reconstructDecay("eta':taulfv2 -> pi+:all pi-:all eta:taulfv", "0.8 < M < 1.1", path=path)
        ma.copyLists("eta':taulfv", ["eta':taulfv1", "eta':taulfv2"], path=path)
        # omega
        ma.reconstructDecay("omega:taulfv -> pi0:taulfv pi-:all pi+:all", "0.73 < M < 0.83", path=path)

        # Lambda0
        ma.reconstructDecay("Lambda0:taulfv_p -> p+:all pi-:all", "0.9 < M < 1.3", path=path)
        ma.reconstructDecay("Lambda0:taulfv_antip -> anti-p-:all pi+:all", "0.9 < M < 1.3", path=path)

        # event selection
#        vm.addAlias("netChargeLFV", "formula(countInList(pi+:taulfv, charge == 1) - countInList(pi+:taulfv, charge == -1))")
        tauLFVCuts1 = "nParticlesInList(pi+:taulfv) < 5 and 1.0 < M < 2.0 and -1.5 < deltaE < 0.5"
        tauLFVCuts3 = "nParticlesInList(pi+:taulfv) < 7 and 1.4 < M < 2.0 and -1.0 < deltaE < 0.5"

        tau_lgamma_Channels = ["e+:taulfv gamma:taulfv",
                               "mu+:taulfv gamma:taulfv"
                               ]

        tau_lll_Channels = ["e+:taulfv e+:taulfv e-:taulfv",
                            "mu+:taulfv mu+:taulfv mu-:taulfv",
                            "mu+:taulfv e+:taulfv e-:taulfv",
                            "e+:taulfv mu+:taulfv mu-:taulfv",
                            "e+:taulfv e+:taulfv mu-:taulfv",
                            "mu+:taulfv mu+:taulfv e-:taulfv"
                            ]

        tau_lP01_Channels = ["e+:taulfv pi0:taulfv",
                             "mu+:taulfv pi0:taulfv",
                             "e+:taulfv eta:taulfv1",
                             "mu+:taulfv eta:taulfv1"
                             ]
        tau_lP03_Channels = ["e+:taulfv eta:taulfv2",
                             "mu+:taulfv eta:taulfv2",
                             "e+:taulfv eta':taulfv",
                             "mu+:taulfv eta':taulfv",
                             "e+:taulfv K_S0:taulfv",
                             "mu+:taulfv K_S0:taulfv"
                             ]

        tau_lS0_Channels = ["e+:taulfv f_0:all",
                            "mu+:taulfv f_0:all"
                            ]

        tau_lV0_Channels = ["e+:taulfv rho0:all",
                            "mu+:taulfv rho0:all",
                            "e+:taulfv K*0:all",
                            "mu+:taulfv K*0:all",
                            "e+:taulfv anti-K*0:all",
                            "mu+:taulfv anti-K*0:all",
                            "e+:taulfv phi:all",
                            "mu+:taulfv phi:all",
                            "e+:taulfv omega:taulfv",
                            "mu+:taulfv omega:taulfv"
                            ]

        tau_lhh_Channels = ["e+:taulfv pi-:taulfv pi+:taulfv",
                            "mu+:taulfv pi-:taulfv pi+:taulfv",
                            "e-:taulfv pi+:taulfv pi+:taulfv",
                            "mu-:taulfv pi+:taulfv pi+:taulfv",
                            "e+:taulfv K-:taulfv K+:taulfv",
                            "mu+:taulfv K-:taulfv K+:taulfv",
                            "e-:taulfv K+:taulfv K+:taulfv",
                            "mu-:taulfv K+:taulfv K+:taulfv",
                            "e+:taulfv K-:taulfv pi+:taulfv",
                            "mu+:taulfv K-:taulfv pi+:taulfv",
                            "e-:taulfv K+:taulfv pi+:taulfv",
                            "mu-:taulfv K+:taulfv pi+:taulfv",
                            "e+:taulfv K_S0:taulfv K_S0:taulfv",
                            "mu+:taulfv K_S0:taulfv K_S0:taulfv"
                            ]

        tau_bnv_Channels = ["mu+:taulfv mu+:taulfv anti-p-:taulfv",
                            "mu-:taulfv mu+:taulfv p+:taulfv",
                            "anti-p-:taulfv pi+:taulfv pi+:taulfv",
                            "p+:taulfv pi-:taulfv pi+:taulfv",
                            "anti-p-:taulfv pi+:taulfv K+:taulfv",
                            "p+:taulfv pi-:taulfv K+:taulfv",
                            "Lambda0:taulfv_p pi+:taulfv",
                            "Lambda0:taulfv_antip pi+:taulfv"
                            ]

        tau_lgamma_list = []
        for chID, channel in enumerate(tau_lgamma_Channels):
            ma.reconstructDecay("tau+:LFV_lgamma" + str(chID) + " -> " + channel, tauLFVCuts1, chID, path=path)
            tau_lgamma_list.append("tau+:LFV_lgamma" + str(chID))

        tau_lll_list = []
        for chID, channel in enumerate(tau_lll_Channels):
            ma.reconstructDecay("tau+:LFV_lll" + str(chID) + " -> " + channel, tauLFVCuts3, chID, path=path)
            tau_lll_list.append("tau+:LFV_lll" + str(chID))

        tau_lP0_list = []
        for chID, channel in enumerate(tau_lP01_Channels):
            ma.reconstructDecay("tau+:LFV_lP01" + str(chID) + " -> " + channel, tauLFVCuts1, chID, path=path)
            tau_lP0_list.append("tau+:LFV_lP01" + str(chID))
        for chID, channel in enumerate(tau_lP03_Channels):
            ma.reconstructDecay("tau+:LFV_lP03" + str(chID) + " -> " + channel, tauLFVCuts3, chID, path=path)
            tau_lP0_list.append("tau+:LFV_lP03" + str(chID))

        tau_lS0_list = []
        for chID, channel in enumerate(tau_lS0_Channels):
            ma.reconstructDecay("tau+:LFV_lS0" + str(chID) + " -> " + channel, tauLFVCuts3, chID, path=path)
            tau_lS0_list.append("tau+:LFV_lS0" + str(chID))

        tau_lV0_list = []
        for chID, channel in enumerate(tau_lV0_Channels):
            ma.reconstructDecay("tau+:LFV_lV0" + str(chID) + " -> " + channel, tauLFVCuts3, chID, path=path)
            tau_lV0_list.append("tau+:LFV_lV0" + str(chID))

        tau_lhh_list = []
        for chID, channel in enumerate(tau_lhh_Channels):
            ma.reconstructDecay("tau+:LFV_lhh" + str(chID) + " -> " + channel, tauLFVCuts3, chID, path=path)
            tau_lhh_list.append("tau+:LFV_lhh" + str(chID))

        tau_bnv_list = []
        for chID, channel in enumerate(tau_bnv_Channels):
            ma.reconstructDecay("tau+:LFV_bnv" + str(chID) + " -> " + channel, tauLFVCuts3, chID, path=path)
            tau_bnv_list.append("tau+:LFV_bnv" + str(chID))

        return tau_lgamma_list + tau_lll_list + tau_lP0_list + tau_lS0_list + tau_lV0_list + tau_lhh_list + tau_bnv_list

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.copyLists('tau+:LFV', self.SkimLists, path=path)

        # add contact information to histogram
        contact = "kenji@hepl.phys.nagoya-u.ac.jp"

        # the variables that are printed out are: M, deltaE
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='tau+:LFV',
            variables_1d=[
                ('M', 100, 1.00, 2.00, '', contact, '', ''),
                ('deltaE', 120, -1.6, 0.6, '', contact, '', '')],
            variables_2d=[('M', 50, 1.00, 2.00, 'deltaE', 60, -1.6, 0.6, '', contact, '', '')],
            path=path)


@fancy_skim_header
class TauGeneric(BaseSkim):
    """
    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-`

    **Criteria**:

    Ntrk = 2

    1. Number of good tracks = 2, net charge < 2
    2. ``visibleEnergyOfEventCMS < 10 GeV and E_ECLtrk < 6 GeV`` and ``missingMomentumOfEvent_theta < 2.6180``
    3. ``visibleEnergyOfEventCMS > 3 GeV or max P_t > 1 GeV``
    4. max. opening angle < 178 deg.

    Ntrk = 3,4

    1. Number of good tracks = 3 or 4, net charge < 2
    2. ``visibleEnergyOfEventCMS < 10.5 GeV and E_ECLtrk < 6 GeV``
    3. ``visibleEnergyOfEventCMS > 3 GeV or max P_t > 1 GeV``
    4. max. opening angle < 178 deg.
    5. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
    6. ``M_tag < 1.8 GeV`` and ``M_sig < 2.3 GeV``

    Ntrk = 5,6

    1. Number of good tracks = 5 or 6, net charge < 2
    2. Event divided by thrust axis; No. good tracks in tag side = 1 or 3
    3. ``M_tag < 1.8 GeV`` and ``M_sig < 2.3 GeV``
    """
    __authors__ = ["Kenji Inami"]
    __description__ = "Skim for Tau generic decays."
    __contact__ = __liaison__
    __category__ = "physics, tau"

    ApplyHLTHadronCut = False
    produce_on_tau_samples = False  # retention is too high on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def additional_setup(self, path):
        """
        Set particle lists and variables for TauGeneric skim.

        **Output particle lists**:
        * ``pi+:tauskim, pi0:tauskim, gamma:pi0_tauskim, gamma:nonpi0_tauskim, gamma:tauskim, pi+:S1/S2, pi0:S1/S2, gamma:S1/S2``

        **Variables**:

        * ``nGoodTracks``: number of good tracks in an event
        * ``netCharge``: total net charge of good tracks
        * ``nTracksS1/nTracksS2:`` number of good tracks in each hemisphere ``S1/S2`` divided by thrust axis
        * ``invMS1/invMS2``: invariant mass of particles in each hemisphere
        * ``maxPt``: maximum Pt among good tracks
        * ``E_ECLtrk``: total ECL energy of good tracks
        """

        tauskim_particle_selection("tauskim", path)

        # Get EventShape variables
        ma.buildEventShape(["pi+:tauskim", "gamma:tauskim"],
                           allMoments=False, foxWolfram=False, cleoCones=False,
                           sphericity=False, jets=False, path=path)
        ma.buildEventKinematics(["pi+:tauskim", "gamma:tauskim"], path=path)

        # Split in signal and tag
        ma.cutAndCopyList("pi+:S1", "pi+:tauskim", "cosToThrustOfEvent > 0", path=path)
        ma.cutAndCopyList("pi+:S2", "pi+:tauskim", "cosToThrustOfEvent < 0", path=path)
        ma.cutAndCopyList("pi0:S1", "pi0:tauskim", "cosToThrustOfEvent > 0", path=path)
        ma.cutAndCopyList("pi0:S2", "pi0:tauskim", "cosToThrustOfEvent < 0", path=path)
        ma.cutAndCopyList("gamma:S1", "gamma:tauskim", "cosToThrustOfEvent > 0", path=path)
        ma.cutAndCopyList("gamma:S2", "gamma:tauskim", "cosToThrustOfEvent < 0", path=path)

        vm.addAlias("nGoodTracks", "nParticlesInList(pi+:tauskim)")
        vm.addAlias("netCharge", "formula(countInList(pi+:tauskim, charge == 1) - countInList(pi+:tauskim, charge == -1))")
        vm.addAlias("nTracksS1", "nParticlesInList(pi+:S1)")
        vm.addAlias("nTracksS2", "nParticlesInList(pi+:S2)")
        vm.addAlias("invMS1", "invMassInLists(pi+:S1, pi0:S1, gamma:S1)")
        vm.addAlias("invMS2", "invMassInLists(pi+:S2, pi0:S2, gamma:S2)")
        # vm.addAlias("Evis", "visibleEnergyOfEventCMS")
        vm.addAlias("maxPt", "maxPtInList(pi+:tauskim)")
        vm.addAlias("E_ECLtrk", "formula(totalECLEnergyOfParticlesInList(pi+:tauskim))")
        vm.addAlias("maxOp", "useCMSFrame(maxOpeningAngleInList(pi+:tauskim))")

    def build_lists(self, path):
        # reconstruct with each Ntrk case
        ma.reconstructDecay('tau+:g2 -> pi+:S1', 'nGoodTracks == 2 and -2 < netCharge <2', path=path)
        ma.reconstructDecay('tau+:g34 -> pi+:S1', '[nGoodTracks == 3 or nGoodTracks == 4] and -2 < netCharge <2', path=path)
        ma.reconstructDecay('tau+:g56 -> pi+:S1', '[nGoodTracks == 5 or nGoodTracks == 6] and -2 < netCharge <2', path=path)

        # Selection criteria
        # Ntrk=2
        ma.applyCuts('tau+:g2', 'visibleEnergyOfEventCMS < 10', path=path)
        ma.applyCuts('tau+:g2', 'E_ECLtrk < 6', path=path)
        ma.applyCuts('tau+:g2', 'missingMomentumOfEvent_theta < 2.6180', path=path)
        ma.applyCuts('tau+:g2', 'visibleEnergyOfEventCMS > 3 or maxPt > 1', path=path)
        ma.applyCuts('tau+:g2', 'maxOp < 3.106686', path=path)
        # Ntrk=3,4
        ma.applyCuts('tau+:g34', 'visibleEnergyOfEventCMS < 10.5', path=path)
        ma.applyCuts('tau+:g34', 'E_ECLtrk < 6', path=path)
        ma.applyCuts('tau+:g34', 'visibleEnergyOfEventCMS > 3 or maxPt > 1', path=path)
        ma.applyCuts('tau+:g34', 'maxOp < 3.106686', path=path)
        ma.applyCuts('tau+:g34',
                     '[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 and invMS2 < 2.3 ] or '
                     '[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 and invMS1 < 2.3 ]', path=path)
        # Ntrk=5,6
        ma.applyCuts('tau+:g56',
                     '[[ nTracksS1 == 1 or nTracksS1 == 3 ] and invMS1 < 1.8 and invMS2 < 2.3 ] or '
                     '[[ nTracksS2 == 1 or nTracksS2 == 3 ] and invMS2 < 1.8 and invMS1 < 2.3 ]', path=path)

        # For skimming, the important thing is if the final particleList is empty or not.
        return ['tau+:g2', 'tau+:g34', 'tau+:g56']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms
        from ROOT import Belle2

        vm.addAlias('Theta_miss', f'formula(missingMomentumOfEvent_theta/{Belle2.Unit.deg})')

        # add contact information to histogram
        contact = "kenji@hepl.phys.nagoya-u.ac.jp"

        ma.copyLists('tau+:generic', self.SkimLists, path=path)
        path = self.skim_event_cuts(cut='nParticlesInList(tau+:generic) > 0', path=path)

        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='',
            variables_1d=[
                ('nGoodTracks', 7, 1, 8, '', contact, '', ''),
                ('visibleEnergyOfEventCMS', 40, 0, 12, '', contact, '', ''),
                ('E_ECLtrk', 70, 0, 7, '', contact, '', ''),
                ('maxPt', 30, 0, 6, '', contact, '', ''),
                ('invMS1', 60, 0, 3, '', contact, '', '', '', ''),
                ('invMS2', 60, 0, 3, '', contact, '', ''),
                ('Theta_miss', 30, 0, 180, '', contact, '', '')],
            variables_2d=[('invMS1', 30, 0, 3, 'invMS2', 30, 0, 3, '', contact, '', '')],
            path=path)


@fancy_skim_header
class TauThrust(BaseSkim):
    """
    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-`

    **Criteria**:

    * ``1 < No. good tracks < 7``
    * ``net charge == 0``
    * Event divided by thrust axis; select 1x1, 1x3, 1x5, 3x3 topology
    * ``0.8 < thrust``
    * ``visibleEnergyOfEventCMS < 10.4 GeV``
    * For 1x1 topology, ``thrust < 0.99``
    * For 1x1 topology, ``1.5 < visibleEnergyOfEventCMS``
    """
    __authors__ = ["Ami Rostomyan", "Kenji Inami"]
    __description__ = "Skim for Tau decays using thrust."
    __contact__ = __liaison__
    __category__ = "physics, tau"

    ApplyHLTHadronCut = False
    produce_on_tau_samples = False  # retention is too high on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdPhotons("all", path=path)

    def additional_setup(self, path):
        """
        Set particle lists and variables for TauThrust skim.

        **Constructed particle lists**:
        * ``pi+:thrust, pi0:thust, gamma:pi0_thrust, gamma:nonpi0_thrust, gamma:thrust, pi+:thrustS1/thrustS2``

        **Variables**:

        * ``nGoodTracksThrust``: number of good tracks in an event
        * ``netChargeThrust``: total net charge of good tracks
        * ``nTracksS1Thrust/nTracksS2Thrust``: number of good tracks in each hemisphere S1/S2 divided by thrust axis
        """

        tauskim_particle_selection("thrust", path)

        # Get EventShape variables
        ma.buildEventShape(['pi+:thrust', 'gamma:thrust'],
                           allMoments=False, foxWolfram=False, cleoCones=False,
                           sphericity=False, jets=False, path=path)
        ma.buildEventKinematics(['pi+:thrust', 'gamma:thrust'], path=path)

        # Split in signal and tag
        ma.cutAndCopyList('pi+:thrustS1', 'pi+:thrust', 'cosToThrustOfEvent > 0', path=path)
        ma.cutAndCopyList('pi+:thrustS2', 'pi+:thrust', 'cosToThrustOfEvent < 0', path=path)

        vm.addAlias('nGoodTracksThrust', 'nParticlesInList(pi+:thrust)')
        vm.addAlias('netChargeThrust', 'formula(countInList(pi+:thrust, charge == 1) - countInList(pi+:thrust, charge == -1))')
        vm.addAlias('nTracksS1Thrust', 'nParticlesInList(pi+:thrustS1)')
        vm.addAlias('nTracksS2Thrust', 'nParticlesInList(pi+:thrustS2)')

    def build_lists(self, path):
        ma.reconstructDecay("tau+:thrust -> pi+:thrustS1", "", path=path)
        eventParticle = ["tau+:thrust"]

        # Selection criteria
        ma.applyCuts("tau+:thrust", "1 < nGoodTracksThrust < 7", path=path)  # cut1
        ma.applyCuts("tau+:thrust", "netChargeThrust == 0", path=path)  # cut2

        topologyCuts = "[nTracksS1Thrust == 1 and nTracksS2Thrust == 1]"  # 1x1
        topologyCuts += " or [nTracksS1Thrust == 1 and nTracksS2Thrust == 3]"\
            " or [nTracksS1Thrust == 3 and nTracksS2Thrust == 1]"  # 1x3, 3x1
        topologyCuts += " or [nTracksS1Thrust == 1 and nTracksS2Thrust == 5]"\
            " or [nTracksS1Thrust == 5 and nTracksS2Thrust == 1]"  # 1x5, 5x1
        topologyCuts += " or [nTracksS1Thrust == 3 and nTracksS2Thrust == 3]"  # 3x3

        ma.applyCuts("tau+:thrust", topologyCuts, path=path)  # cut3
        ma.applyCuts("tau+:thrust", "0.8 < thrust", path=path)  # cut4
        ma.applyCuts("tau+:thrust", "visibleEnergyOfEventCMS < 10.4", path=path)  # cut5
        # cut for 1x1 topology
        ma.applyCuts("tau+:thrust", "thrust < 0.99 or nGoodTracksThrust!=2", path=path)
        ma.applyCuts("tau+:thrust", "1.5 < visibleEnergyOfEventCMS or nGoodTracksThrust!=2", path=path)

        return eventParticle

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        path = self.skim_event_cuts(cut='nParticlesInList(tau+:thrust) > 0', path=path)

        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='',
            variables_1d=[
                ('nGoodTracksThrust', 7, 1, 8, '', self.__contact__, '', ''),
                ('visibleEnergyOfEventCMS', 40, 0, 12, '', self.__contact__, '', ''),
                ('thrust', 50, 0.8, 1, '', self.__contact__, '', '')],
            path=path)


@fancy_skim_header
class TauToMuMuMu(BaseSkim):
    """
    **Channel**: :math:`\\tau \\to \\mu \\mu \\mu and \\tau \\to \\pi \\pi \\pi for control sample`

    **Output particle lists**: ``mu+:taulfv, pi+:control``

    **Criteria for 3mu states**: Number of good tracks < 7, :math:`1.4 < M < 2.0` GeV, :math:`-1.0 < \\Delta E < 0.5` GeV

    **Criteria for 3pi states**: Number of good tracks < 7, :math:`0.5 < M < 2.0` GeV, :math:`-1.0 < \\Delta E < 0.5` GeV
    """
    __authors__ = ["Junewoo PARK"]
    __description__ = "Skim for Tau 3mu decays."
    __contact__ = __liaison__
    __category__ = "physics, tau"

    ApplyHLTHadronCut = False
    produce_on_tau_samples = True  # retention is ~1.7% on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdMu("all", path=path)
        stdPi("all", path=path)

    def build_lists(self, path):
        # particle selection
        trackCuts = "[-3.0 < dz < 3.0] and [dr < 1.0]"
        ma.cutAndCopyList("pi+:goodtrack", "pi+:all", trackCuts, path=path)
        ma.cutAndCopyList("pi+:control", "pi+:all", trackCuts + ' and [pionID > 0.9]', path=path)

        # reconstruct tau->mumumu
        ma.reconstructDecay(
            decayString="tau+:mumumu -> mu+:all mu+:all mu-:all",
            cut="[nParticlesInList(pi+:goodtrack) < 7] and [1.4 < M < 2.0] and [-1.0 < deltaE < 0.5]",
            dmID=0,
            path=path)
        Condition_one = '[[daughter(0, p)>daughter(1, p)] and [daughter(0, p)>daughter(2, p)] and [daughter(0, muonID) > 0.1]]'
        Condition_two = '[[daughter(1, p)>daughter(0, p)] and [daughter(1, p)>daughter(2, p)] and [daughter(1, muonID) > 0.1]]'
        Condition_three = '[[daughter(2, p)>daughter(0, p)] and [daughter(2, p)>daughter(1, p)] and [daughter(2, muonID) > 0.1]]'
        ma.applyCuts('tau+:mumumu', Condition_one + ' or ' + Condition_two + ' or ' + Condition_three, path=path)

        # reconstruct tau->pipipi
        ma.reconstructDecay(
            decayString="tau+:control -> pi+:control pi+:control pi-:control",
            cut="[nParticlesInList(pi+:goodtrack) < 7] and [0.5 < M < 2.0] and [-1.0 < deltaE < 0.5]",
            dmID=1,
            path=path)

        # combine list
        ma.copyLists(outputListName="tau+:comb", inputListNames=["tau+:mumumu", "tau+:control"], path=path)

        return ["tau+:comb"]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        ma.copyLists('tau+:comb', self.SkimLists, path=path)

        # add contact information to histogram
        contact = "	junewoo@g.ecc.u-tokyo.ac.jp"

        # the variables that are printed out are: M, deltaE
        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist='tau+:comb',
            variables_1d=[
                ('M', 100, 1.00, 2.00, '', contact, '', ''),
                ('deltaE', 120, -1.6, 0.6, '', contact, '', '')],
            variables_2d=[('M', 50, 1.00, 2.00, 'deltaE', 60, -1.6, 0.6, '', contact, '', '')],
            path=path)

############################################################


@fancy_skim_header
class TauKshort(BaseSkim):
    """
    **Channel**: :math:`e^+ e^- \\to \\tau^+ \\tau^-, \\tau \\to K_s \\pi X`

    **Criteria**:

    * ``nAlltracks < 7``
    * ``0.9 < thrust < 0.995``
    * ``2.5 < visibleEnergyOfEventCMS < 10 GeV``
    * ``0.2 < track_kshort_pt < 4.5``
    * ``max(kshort_track1_pt,kshort_track2_pt) > 0.25``
    * ``track_kshort_significanceOfDistance > 3``
    """
    __authors__ = ["Paolo Leo", "Kenji Inami"]
    __description__ = "Skim for Tau decays with Kshort."
    __contact__ = __liaison__
    __category__ = "physics, tau"

    ApplyHLTHadronCut = False
    produce_on_tau_samples = False  # retention is too high on taupair
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        # if arg_dataORmc == 'mc':
        #    stdKshorts(prioritiseV0=True, fitter='TreeFit', path=main, updateAllDaughters=True)
        # if arg_dataORmc == 'data':
        #    stdKshorts(prioritiseV0=True, fitter='TreeFit', path=main, updateAllDaughters=False)
        stdKshorts(path=path)

    def additional_setup(self, path):
        """
        Set particle lists and variables for TauKshort skim.

        **Constructed particle lists**:
        * ``pi+:tauKs, pi0:tauKs, gamma:pi0_tauKs, gamma:nonpi0_tauKs, gamma:tauKs, pi+:tauKsS1/tauKsS2``

        **Variables**:

        """

        tauskim_particle_selection("tauKs", path)

    def build_lists(self, path):

        vm.addAlias('nAllTracksTauKs', 'nParticlesInList(pi+:all)')

        ma.cutAndCopyLists('K_S0:tauKs_merged', 'K_S0:merged', 'significanceOfDistance > 3', path=path)
        ma.cutAndCopyLists('pi+:tauKs_kshort', 'pi+:all', 'isDescendantOfList(K_S0:tauKs_merged) == 1', path=path)
        ma.cutAndCopyLists('pi+:tauKs_notKs', 'pi+:tauKs', 'isDescendantOfList(K_S0:tauKs_merged) == 0', path=path)

        ma.copyLists('pi+:tauKs_used', ['pi+:tauKs_kshort', 'pi+:tauKs_notKs'], path=path)
        vm.addAlias('netChargeTauKs',
                    'formula(countInList(pi+:tauKs_used, charge == 1) - countInList(pi+:tauKs_used, charge == -1))')

        # Get EventShape variables
        ma.buildEventShape(
            ['pi+:tauKs_notKs', 'pi+:tauKs_kshort', 'gamma:tauKs'],
            allMoments=False, foxWolfram=False, cleoCones=False,
            sphericity=False, jets=False, path=path)
        ma.buildEventKinematics(
            ['pi+:tauKs_notKs', 'pi+:tauKs_kshort', 'gamma:tauKs'],
            path=path)

        # reconstruct
        ma.reconstructDecay('tau+:tauKs_kshort -> K_S0:tauKs_merged pi+:tauKs_notKs', '', path=path)
        ma.reconstructDecay('tau-:tauKs_1prong -> pi-:tauKs', '', path=path)
        ma.reconstructDecay('vpho:tauKs -> tau+:tauKs_kshort tau-:tauKs_1prong', '', path=path)
        # Cut on events requiring opposite hemispheres
        vm.addAlias('prod1',
                    'formula(daughter(0, daughter(0, cosToThrustOfEvent))*daughter(1, daughter(0,cosToThrustOfEvent)))')
        vm.addAlias('prod2',
                    'formula(daughter(0, daughter(1, cosToThrustOfEvent))*daughter(1, daughter(0,cosToThrustOfEvent)))')
        ma.applyCuts('vpho:tauKs', 'prod1 < 0 and prod2 < 0', path=path)

        evP = 'vpho:tauKs'
        ma.applyCuts(evP, 'nAllTracksTauKs < 7', path=path)   # cut0
        ma.applyCuts(evP, '0.9 < thrust < 0.995', path=path)  # cut1
        ma.applyCuts(evP, '2.5 < visibleEnergyOfEventCMS < 10', path=path)  # cut2
        ####
        commonVariables = vc.kinematics
        kshortVariables = ['flightDistance', 'significanceOfDistance', 'daughter1DecayAngle', 'daughter2DecayAngle']
        vu.create_aliases_for_selected(list_of_variables=commonVariables + kshortVariables,
                                       decay_string='vpho -> [tau+ -> ^K_S0 ^pi+] [tau- -> ^pi-]',
                                       prefix=['track_kshort', 'track_pi_kshort', 'track_1prong'])
        vu.create_aliases_for_selected(list_of_variables=commonVariables,
                                       decay_string='vpho -> [tau+ -> [K_S0 -> ^pi+ ^pi-] pi+] tau-',
                                       prefix=['kshort_track1', 'kshort_track2'])
        ####
        ma.applyCuts(evP, '0.2 < track_kshort_pt < 4.5', path=path)   # cut3
        ma.applyCuts(evP, 'max(kshort_track1_pt, kshort_track2_pt) > 0.25', path=path)   # cut4
        ma.applyCuts(evP, 'track_kshort_significanceOfDistance > 3', path=path)   # cut5

        return [evP]

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import create_validation_histograms

        contact = "kenji@hepl.phys.nagoya-u.ac.jp"

        evP = 'vpho:tauKs'
        ma.rankByHighest(particleList=evP, variable='p', numBest=1, path=path)

        # ma.fillParticleListsFromMC([('K_S0:gen','')],path=path)
        # vm.addAlias('nKsMC', 'nParticlesInList(K_S0:gen)')

        create_validation_histograms(
            rootfile=f'{self}_Validation.root',
            particlelist=evP,
            variables_1d=[
                # ('nKsMC',5,0,5,'', contact, '', ''),
                ('nAllTracksTauKs', 7, 1, 8, '', contact, '', ''),
                ('netChargeTauKs', 7, -3, 4, '', contact, '', ''),
                ('visibleEnergyOfEventCMS', 40, 0, 12, '', contact, '', ''),
                ('thrust', 50, 0.75, 1, '', contact, '', '')],
            path=path)
