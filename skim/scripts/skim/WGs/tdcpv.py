#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma
from skim.standardlists.dileptons import (loadStdJpsiToee, loadStdJpsiTomumu,
                                          loadStdPsi2s2lepton)
from skim.standardlists.lightmesons import (loadStdSkimHighEffTracks,
                                            loadStdSkimHighEffPhi, loadStdSkimHighEffEtaPrime,
                                            loadStdSkimHighEffEta, loadStdSkimHighEffKstar0,
                                            loadStdSkimHighEffRho0, loadStdSkimHighEffOmega,
                                            loadStdSkimHighEffF_0, loadStdAllRho0)

from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons
from stdPi0s import loadStdSkimPi0, stdPi0s, loadStdSkimHighEffPi0
from stdV0s import stdKshorts
from variables import variables as vm
from stdKlongs import stdKlongs

__authors__ = [
    "Chiara La Licata <chiara.lalicata@ts.infn.it>",
    "Stefano Lacaprara  <stefano.lacaprara@pd.infn.it>"
]

__liaison__ = "Noah BRENNY <nbrenny@iastate.edu>"
_VALIDATION_SAMPLE = "mdst16.root"


@fancy_skim_header
class TDCPV_qqs(BaseSkim):
    """
    **Physics channels**: bd/u → qqs

    **Decay Channels**:

    * :math:`B^0 \\to \\phi K_S^0`
    * :math:`B^0 \\to \\phi K^{*0}`
    * :math:`B^0 \\to \\eta K_S^0`
    * :math:`B^0 \\to \\eta' K_S^0`
    * :math:`B^0 \\to \\eta K^{*0}`
    * :math:`B^0 \\to \\eta' K^{*0}`
    * :math:`B^0 \\to K_S^0 K_S^0 K_S^0`
    * :math:`B^0 \\to \\pi^0 K_S^0`
    * :math:`B^0 \\to \\rho^0 K_S^0`
    * :math:`B^0 \\to \\omega K_S^0`
    * :math:`B^0 \\to f_0 K_S^0`
    * :math:`B^0 \\to \\pi^0 \\pi^0 K_S^0`
    * :math:`B^0 \\to \\phi K_S^0 \\pi^0`
    * :math:`B^0 \\to \\pi^+ \\pi^- K_S^0`
    * :math:`B^0 \\to \\pi^+ \\pi^- K_S^0 \\gamma`
    * :math:`B^0 \\to \\pi^0 K_S^0 \\gamma`
    * :math:`B^0 \\to \\phi K_S^0 \\gamma`
    * :math:`B^0 \\to \\eta K_S^0 \\gamma`
    * :math:`B^0 \\to \\rho^0 \\gamma`
    * :math:`B^0 \\to \\omega \\gamma`
    * :math:`B^0 \\to \\phi \\gamma`

    * :math:`B^+ \\to \\eta' K^+`
    * :math:`B^+ \\to \\phi K^+`
    * :math:`B^+ \\to \\phi K^{*+}`
    * :math:`B^+ \\to \\omega K^+`
    * :math:`B^+ \\to \\rho^0 K^+`
    * :math:`B^+ \\to K_S^0 K_S^0 K^+`
    * :math:`B^+ \\to \\pi^+ \\pi^- K^+ \\gamma`

    **Particle lists used**:

    * ``phi:SkimHighEff``
    * ``eta':SkimHighEff``
    * ``eta:SkimHighEff``
    * ``pi0:eff40_May2020``
    * ``pi0:skim``
    * ``pi0:SkimHighEff``
    * ``rho0:SkimHighEff``
    * ``omega:SkimHighEff``
    * ``f_0:SkimHighEff``
    * ``pi+:SkimHighEff``
    * ``K+:SkimHighEff``
    * ``omega:SkimHighEff``
    * ``K*0:SkimHighEff``
    * ``gamma:E15 , cut : 1.4 < E < 4``
    * ``gamma:ECMS16 , cut : 1.6 < useCMSFrame(E)``
    * ``K_S0:merged``
    * ``K+:1%``

    **Cuts used**:

    * SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01
    * 5.2 < Mbc < 5.29
    * abs(deltaE) < 0.5
    * nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1
    * E_ECL_TDCPV < 9
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata", "Stefano Lacaprara"]
    __description__ = "Skim for time-dependent CP violation analysis b->qqs decays"
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        stdPhotons("tight", path=path)
        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)
        loadStdSkimPi0(path=path)
        loadStdSkimHighEffPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_May2020", path=path)
        loadStdAllRho0(path=path)

        loadStdSkimHighEffPhi(path=path)
        loadStdSkimHighEffEta(path=path)
        loadStdSkimHighEffEtaPrime(path=path)
        loadStdSkimHighEffKstar0(path=path)
        loadStdSkimHighEffRho0(path=path)
        loadStdSkimHighEffOmega(path=path)
        loadStdSkimHighEffF_0(path=path)

        # Additional non-standard lists
        ma.reconstructDecay('phi:SkimHighEff2_TDCPV_qqs -> pi0:skim pi-:SkimHighEff pi+:SkimHighEff', '0.97 < M < 1.1', path=path)
        ma.reconstructDecay('K_S0:pi0pi0_TDCPV_qqs -> pi0:skim pi0:skim', '0.4 < M < 0.6', path=path)

        ma.cutAndCopyList('pi0:SkimHighEffCut_TDCPV_qqs', 'pi0:SkimHighEff', 'M > 0.105 and M < 0.150', path=path)

        ma.reconstructDecay('K*+:kshort_pip_TDCPV_qqs -> K_S0:merged pi+:SkimHighEff', '0.74 < M < 1.04', path=path)
        ma.reconstructDecay('K*+:kp_piz_TDCPV_qqs -> K+:SkimHighEff pi0:SkimHighEffCut_TDCPV_qqs', '0.74 < M < 1.04', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15_TDCPV_qqs', 'gamma:all', '1.4<E<4', path=path)
        ma.cutAndCopyList('gamma:ECMS16_TDCPV_qqs', 'gamma:all', '1.6<useCMSFrame(E)', path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV_qqs', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_qqs_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV_qqs', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_qqs_eventshape)')
        vm.addAlias('E_ECL_TDCPV_qqs', 'formula(E_ECL_pi_TDCPV_qqs+E_ECL_gamma_TDCPV_qqs)')

        btotcpvcuts = '5.2 < Mbc and abs(deltaE) < 0.5'

        bd_qqs_Channels = [
            'phi:SkimHighEff K_S0:merged',
            'phi:SkimHighEff K*0:SkimHighEff',
            'eta\':SkimHighEff K_S0:merged',
            'eta:SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K*0:SkimHighEff',
            'eta:SkimHighEff K*0:SkimHighEff',
            'K_S0:merged K_S0:merged K_S0:merged',
            'pi0:SkimHighEffCut_TDCPV_qqs K_S0:merged',
            'rho0:SkimHighEff K_S0:merged',
            'omega:SkimHighEff K_S0:merged',
            'f_0:SkimHighEff K_S0:merged',
            'pi0:skim pi0:skim K_S0:merged',
            'phi:SkimHighEff K_S0:merged pi0:skim',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged gamma:E15_TDCPV_qqs',
            'pi0:skim K_S0:merged gamma:E15_TDCPV_qqs',
            'pi0:SkimHighEff K_S0:merged gamma:ECMS16_TDCPV_qqs',
            'phi:SkimHighEff2_TDCPV_qqs K_S0:merged',
            'phi:SkimHighEff K_S0:pi0pi0_TDCPV_qqs',
            'eta\':SkimHighEff K_S0:pi0pi0_TDCPV_qqs',
            'phi:SkimHighEff K_S0:merged gamma:E15_TDCPV_qqs',
            'eta:SkimHighEff K_S0:merged gamma:E15_TDCPV_qqs',
            'rho0:SkimHighEff gamma:E15_TDCPV_qqs',
            'omega:SkimHighEff gamma:E15_TDCPV_qqs',
            'phi:SkimHighEff gamma:E15_TDCPV_qqs'
        ]

        bu_qqs_Channels = [
            'eta\':SkimHighEff K+:SkimHighEff',
            'phi:SkimHighEff K+:SkimHighEff',
            'phi:SkimHighEff K*+:kshort_pip_TDCPV_qqs',
            'phi:SkimHighEff K*+:kp_piz_TDCPV_qqs',
            'omega:SkimHighEff K+:SkimHighEff',
            'rho0:SkimHighEff K+:SkimHighEff',
            'K_S0:merged K_S0:merged K+:SkimHighEff',
            'pi+:SkimHighEff pi-:SkimHighEff K+:SkimHighEff gamma:E15_TDCPV_qqs'
        ]

        bd_qqs_List = []
        for chID, channel in enumerate(bd_qqs_Channels):
            ma.reconstructDecay('B0:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bd_qqs_List.append('B0:TDCPV_qqs' + str(chID))

        bu_qqs_List = []
        for chID, channel in enumerate(bu_qqs_Channels):
            ma.reconstructDecay('B+:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bu_qqs_List.append('B+:TDCPV_qqs' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_qqs_eventshape',
                            cut='pt > 0.1 and abs(dr)<0.5 and abs(dz)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_qqs_eventshape',
                            cut='E > 0.1 and thetaInCDCAcceptance', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_qqs_eventshape', 'gamma:TDCPV_qqs_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_qqs_eventshape', 'gamma:TDCPV_qqs_eventshape'], path=path)

        EventCuts = [
            "nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1",
            "E_ECL_TDCPV_qqs<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_qqs_List + bu_qqs_List

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import ValidationMetadataSetter

        ma.reconstructDecay("B0:etap_TDCPV_qqs -> eta':SkimHighEff K_S0:merged", '5.2 < Mbc < 5.3 and abs(deltaE) < 0.3', path=path)

        ma.reconstructDecay("K_10:all_TDCPV_qqs -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:Kspipig_TDCPV_qqs -> K_10:all_TDCPV_qqs gamma:E15_TDCPV_qqs",
                            "Mbc > 5.2 and abs(deltaE) < 0.5", path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        filename = f'{self}_Validation.root'
        metadata = []
        for directory in ["etap", "Kspipig"]:
            metadata.append(['deltaE', directory, '#Delta E', __liaison__,
                            f'Energy difference of B for {directory} mode', '', '#Delta E [GeV]', 'Candidates'])
            metadata.append(['Mbc', directory, 'Mbc', __liaison__,
                            f'Beam-constrained mass for {directory} mode', '', 'M_{bc} [GeV]', 'Candidates'])
        path.add_module(ValidationMetadataSetter(metadata, filename))
        ma.variablesToHistogram('B0:etap_TDCPV_qqs', variableshisto, filename=filename, path=path, directory="etap")
        ma.variablesToHistogram('B0:Kspipig_TDCPV_qqs', variableshisto, filename=filename, path=path, directory="Kspipig")
        variableshisto = [('deltaE', 135, -0.020, 0.250)]


@fancy_skim_header
class TDCPV_klong(BaseSkim):
    """
    **Physics channels**: bd/u → qqs with Klongs

    **Decay Channels**:

    * :math:`B^0 \\to \\phi K_L^0`
    * :math:`B^0 \\to \\eta ' K_L^0`
    * :math:`B^0 \\to \\omega K_L^0`
    * :math:`B^0 \\to \\rho^0 K_L^0`
    * :math:`B^0 \\to \\pi^0 K_L^0`

    **Particle lists used**:

    * ``phi:SkimHighEff``
    * ``eta:SkimHighEff1``
    * ``eta:SkimHighEff2``
    * ``rho0:SkimHighEff``
    * ``K_L0:allklm``
    * ``K_L0:allecl``

    **Cuts used**:

    * SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01
    * clusterE>0.150 for loose ECL KL
    * klmClusterInnermostLayer<=10 and klmClusterLayers<=10 for loose KLM KL
    * clusterPulseShapeDiscriminationMVA<0.15 and clusterE>0.25 for tight ECL KL
    * klmClusterKlId>0.1 and klmClusterInnermostLayer<=10 and klmClusterLayers<=10 for tight KLM KL
    * abs(deltaE) < 0.1
    * nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1,
    * E_ECL_TDCPV < 9
    * foxWolframR2<0.6
    """

    __authors__ = ["Michele Veronesi", "Noah Brenny"]
    __description__ = "Skim for time-dependent CP violation analysis b->qqs decays using Klongs"
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdK("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        stdPhotons("tight", path=path)
        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)
        loadStdSkimPi0(path=path)
        loadStdSkimHighEffPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_May2020", path=path)

        loadStdSkimHighEffPhi(path=path)
        loadStdSkimHighEffEta(path=path)
        loadStdSkimHighEffRho0(path=path)
        loadStdSkimHighEffOmega(path=path)

        # Additional non-standard lists
        ma.reconstructDecay('phi:SkimHighEff2_TDCPV_klong -> pi0:skim pi-:SkimHighEff pi+:SkimHighEff', '0.97 < M < 1.1', path=path)

        ma.cutAndCopyList('eta:SkimKlong2_TDCPV_klong', 'eta:SkimHighEff2', '0.5 < M < 0.6', path=path)

        ma.reconstructDecay('eta\':klong_ggpp_TDCPV_klong -> eta:SkimHighEff1 pi+:SkimHighEff pi-:SkimHighEff',
                            '0.8 < M < 1.1', path=path)
        ma.reconstructDecay(
            'eta\':klong_pipipi_TDCPV_klong -> eta:SkimKlong2_TDCPV_klong pi+:SkimHighEff pi-:SkimHighEff',
            '0.9 < M < 1',
            path=path)
        ma.reconstructDecay('eta\':klong_rhogam_TDCPV_klong -> rho0:SkimHighEff gamma:tight', '0.9 < M < 1', path=path)

        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15_TDCPV_klong', 'gamma:all', '1.4<E<4', path=path)
        ma.cutAndCopyList('gamma:ECMS16_TDCPV_klong', 'gamma:all', '1.6<useCMSFrame(E)', path=path)

        # loose KL
        ma.cutAndCopyList(
            'K_L0:ecl_loose_TDCPV_klong',
            'K_L0:allecl',
            'clusterE>0.150',
            path=path)
        ma.cutAndCopyList(
            'K_L0:klm_loose_TDCPV_klong',
            'K_L0:allklm',
            '[klmClusterInnermostLayer<=10] and [klmClusterLayers<=10]',
            path=path)

        # tight KL
        ma.cutAndCopyList(
            'K_L0:ecl_tight_TDCPV_klong',
            'K_L0:allecl',
            '[clusterPulseShapeDiscriminationMVA<0.15] and [clusterE>0.25]',
            path=path)
        ma.cutAndCopyList(
            'K_L0:klm_tight_TDCPV_klong',
            'K_L0:allklm',
            '[klmClusterKlId>0.1] and [klmClusterInnermostLayer<=10] and [klmClusterLayers<=10]',
            path=path)

        ma.copyLists('K_L0:eclklm_qqs_0', ['K_L0:ecl_loose_TDCPV_klong', 'K_L0:klm_loose_TDCPV_klong'], path=path)  # phi(KK)KL
        ma.copyLists('K_L0:eclklm_qqs_1', ['K_L0:ecl_tight_TDCPV_klong', 'K_L0:klm_tight_TDCPV_klong'], path=path)  # phi(3pi)KL
        ma.copyLists('K_L0:eclklm_qqs_2',
                     ['K_L0:ecl_loose_TDCPV_klong',
                      'K_L0:klm_loose_TDCPV_klong'],
                     path=path)  # eta'(eta(gg)pipi)KL
        ma.copyLists('K_L0:eclklm_qqs_3',
                     ['K_L0:ecl_tight_TDCPV_klong',
                      'K_L0:klm_tight_TDCPV_klong'],
                     path=path)  # eta'(eta(3pi)pipi)KL
        ma.copyLists('K_L0:eclklm_qqs_4',
                     ['K_L0:ecl_tight_TDCPV_klong',
                      'K_L0:klm_tight_TDCPV_klong'],
                     path=path)  # eta'(rho(pipi)gam)KL
        ma.copyLists('K_L0:eclklm_qqs_5', ['K_L0:ecl_tight_TDCPV_klong', 'K_L0:klm_tight_TDCPV_klong'], path=path)  # omegaKL
        ma.copyLists('K_L0:eclklm_qqs_6', ['K_L0:ecl_tight_TDCPV_klong', 'K_L0:klm_tight_TDCPV_klong'], path=path)  # rho0KL
        ma.copyLists('K_L0:eclklm_qqs_7', ['K_L0:ecl_tight_TDCPV_klong', 'K_L0:klm_tight_TDCPV_klong'], path=path)  # pi0KL

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV_klong', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_klong_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV_klong', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_klong_eventshape)')
        vm.addAlias('E_ECL_TDCPV_klong', 'formula(E_ECL_pi_TDCPV_klong+E_ECL_gamma_TDCPV_klong)')

        btotcpvcuts_KL = 'abs(deltaE) < 0.10'

        bd_klong_Channels = [
            'phi:SkimHighEff K_L0:eclklm_qqs_0',
            'phi:SkimHighEff2_TDCPV_klong K_L0:eclklm_qqs_1',
            'eta\':klong_ggpp_TDCPV_klong K_L0:eclklm_qqs_2',
            'eta\':klong_pipipi_TDCPV_klong K_L0:eclklm_qqs_3',
            'eta\':klong_rhogam_TDCPV_klong K_L0:eclklm_qqs_4',
            'omega:SkimHighEff K_L0:eclklm_qqs_5',
            'rho0:SkimHighEff K_L0:eclklm_qqs_6',
            'pi0:skim K_L0:eclklm_qqs_7'
        ]

        bd_klong_List = []

        for chID, channel in enumerate(bd_klong_Channels):
            ma.reconstructMissingKlongDecayExpert('B0:TDCPV_klong' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path)
            bd_klong_List.append('B0:TDCPV_klong' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_klong_eventshape',
                            cut='pt > 0.1 and abs(dr)<0.5 and abs(dz)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_klong_eventshape',
                            cut='E > 0.1 and thetaInCDCAcceptance', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_klong_eventshape', 'gamma:TDCPV_klong_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_klong_eventshape', 'gamma:TDCPV_klong_eventshape'], path=path)

        EventCuts = [
            "nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1",
            "E_ECL_TDCPV_klong<9",
            "foxWolframR2<0.6"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_klong_List


@fancy_skim_header
class TDCPV_ccs(BaseSkim):
    """
    **Physics channels**:  bd → ccs

    **Decay Channels**:

    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) K_S^0`
    * :math:`B^0 \\to \\psi(2S) (ee/\\mu\\mu) K_S^0`
    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) K^* (K^+ \\pi^- / K_S^0 \\pi^0)`
    * :math:`B^+ \\to J/\\psi (ee/\\mu\\mu) K^+`
    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) K_L`
    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) \\eta (\\pi^+ \\pi^- \\pi^0 / \\pi^+ \\pi^-)`
    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) \\pi^0`
    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) K^+ \\pi^-`
    * :math:`B^+ \\to J/\\psi (ee/\\mu\\mu) K^{*+} (\\pi^+ K_S^0 / K^+ \\pi^0)`

    **Particle lists used**:

    * ``K_S0:merged``
    * ``pi+:all``
    * ``J/psi:ee``
    * ``J/psi:mumu``
    * ``psi(2S):ee``
    * ``psi(2S):mumu``
    * ``K*0:SkimHighEff``
    * ``K+:SkimHighEff``
    * ``K_L0:allklm``
    * ``K_L0:allecl``
    * ``eta:SkimHighEff``
    * ``pi0:eff40_May2020``
    * ``pi0:eff60_May2020``

    **Cuts used**:

    * SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01
    * 5.2 < Mbc < 5.29 for Ks/K*
    * abs(deltaE) < 0.3 and 5.05 < Mbc < 5.29 for KL
    * abs(deltaE) < 0.5
    * nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits>20)>=3
    * nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1
    * visibleEnergyOfEventCMS>4
    * E_ECL_TDCPV < 9
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata", "Stefano Lacaprara"]
    __description__ = "Skim for time-dependent CP violation analysis."
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        stdPhotons("tight", path=path)

        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)

        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_May2020", path=path)
        stdPi0s("eff60_May2020", path=path)
        loadStdSkimHighEffKstar0(path=path)
        loadStdSkimHighEffEta(path=path)

        loadStdJpsiToee(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2lepton(path=path)
        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

        ma.reconstructDecay('K*0:neutral_TDCPV_ccs -> K_S0:merged pi0:eff40_May2020', '0.74 < M < 1.04', path=path)
        ma.reconstructDecay('K*+:kshort_pip_TDCPV_ccs -> K_S0:merged pi+:SkimHighEff', '0.74 < M < 1.04', path=path)
        ma.reconstructDecay('K*+:kp_piz_TDCPV_ccs -> K+:SkimHighEff pi0:eff40_May2020', '0.74 < M < 1.04', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('K_L0:alleclEcut_ccs', 'K_L0:allecl', 'clusterE>0.15 and clusterE1E9<0.85', path=path)
        ma.cutAndCopyList(
            'K_L0:klmLayers_ccs',
            'K_L0:allklm',
            '[klmClusterInnermostLayer<=10] and [klmClusterLayers<=7] and [klmClusterKlId>0.001]',
            path=path)
        ma.copyLists('K_L0:all_klmecl_ccs_0', ['K_L0:klmLayers_ccs', 'K_L0:alleclEcut_ccs'], writeOut=True, path=path)
        ma.copyLists('K_L0:all_klmecl_ccs_1', ['K_L0:klmLayers_ccs', 'K_L0:alleclEcut_ccs'], writeOut=True, path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV_ccs', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_ccs_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV_ccs', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_ccs_eventshape)')
        vm.addAlias('E_ECL_TDCPV_ccs', 'formula(E_ECL_pi_TDCPV_ccs+E_ECL_gamma_TDCPV_ccs)')

        btotcpvcuts = 'Mbc > 5.2 and abs(deltaE) < 0.5'
        btotcpvcuts_KL = 'abs(deltaE) < 0.25 and 5.05 < Mbc < 5.29'

        bd_ccs_Channels = ['J/psi:ee K_S0:merged',
                           'J/psi:mumu K_S0:merged',
                           'psi(2S):ll K_S0:merged',
                           'J/psi:ee K*0:SkimHighEff',
                           'J/psi:mumu K*0:SkimHighEff',
                           'J/psi:ee K*0:neutral_TDCPV_ccs',
                           'J/psi:mumu K*0:neutral_TDCPV_ccs',
                           'J/psi:ee eta:SkimHighEff',
                           'J/psi:mumu eta:SkimHighEff',
                           'J/psi:ee pi0:eff60_May2020',
                           'J/psi:mumu pi0:eff60_May2020',
                           'J/psi:ee K+:SkimHighEff pi-:SkimHighEff',
                           'J/psi:mumu K+:SkimHighEff pi-:SkimHighEff']

        bPlustoJPsiK_Channel = ['J/psi:mumu K+:SkimHighEff',
                                'J/psi:ee K+:SkimHighEff']

        bd_ccs_KL_Channels = ['J/psi:mumu K_L0:all_klmecl_ccs_0',
                              'J/psi:ee K_L0:all_klmecl_ccs_1']

        bd_ccs_List = []
        for chID, channel in enumerate(bd_ccs_Channels):
            ma.reconstructDecay('B0:TDCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TDCPV_ccs' + str(chID), 'nTracks>4', path=path)
            bd_ccs_List.append('B0:TDCPV_ccs' + str(chID))

        bPlustoJPsiK_List = []

        for chID, channel in enumerate(bPlustoJPsiK_Channel):
            ma.reconstructDecay('B+:TDCPV_JPsiK' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bPlustoJPsiK_List.append('B+:TDCPV_JPsiK' + str(chID))

        b0toJPsiKL_List = []
        for chID, channel in enumerate(bd_ccs_KL_Channels):
            ma.reconstructMissingKlongDecayExpert('B0:TDCPV_JPsiKL' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path,
                                                  recoList=f'_reco{chID}')
            b0toJPsiKL_List.append('B0:TDCPV_JPsiKL' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_ccs_eventshape',
                            cut='pt > 0.1 and abs(dr)<0.5 and abs(dz)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_ccs_eventshape',
                            cut='E > 0.1 and thetaInCDCAcceptance', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_ccs_eventshape', 'gamma:TDCPV_ccs_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_ccs_eventshape', 'gamma:TDCPV_ccs_eventshape'], path=path)

        EventCuts = [
            "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV_ccs<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_ccs_List + bPlustoJPsiK_List + b0toJPsiKL_List

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import ValidationMetadataSetter

        ma.reconstructDecay('B0:jpsiee_TDCPV_ccs -> J/psi:ee K_S0:merged', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)
        ma.reconstructDecay('B0:jpsimumu_TDCPV_ccs -> J/psi:mumu K_S0:merged', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        filename = f'{self}_Validation.root'
        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        metadata = []
        for directory in ["jpsiee", "jpsimumu", "KLjpsimumu", "KLjpsiee"]:
            metadata.append(['deltaE', directory, '#Delta E', __liaison__,
                            f'Energy difference of B for {directory} mode', '', '#Delta E [GeV]', 'Candidates'])
        for directory in ["jpsiee", "jpsimumu"]:
            metadata.append(['Mbc', directory, 'Mbc', __liaison__,
                            f'Beam-constrained mass for {directory} mode', '', 'M_{bc} [GeV]', 'Candidates'])
        path.add_module(ValidationMetadataSetter(metadata, filename))
        ma.variablesToHistogram('B0:jpsiee_TDCPV_ccs', variableshisto, filename=filename, path=path, directory="jpsiee")
        ma.variablesToHistogram('B0:jpsimumu_TDCPV_ccs', variableshisto, filename=filename, path=path, directory="jpsimumu")

        variableshisto = [('deltaE', 100, -0.020, 0.180)]
        ma.variablesToHistogram('B0:TDCPV_JPsiKL0', variableshisto, filename=filename, path=path, directory="KLjpsimumu")
        ma.variablesToHistogram('B0:TDCPV_JPsiKL1', variableshisto, filename=filename, path=path, directory="KLjpsiee")


@fancy_skim_header
class TDCPV_dilepton(BaseSkim):
    """
    Reconstructed decays
        * :math:`B\\overline{B} \\to l^+l^-`
        * :math:`B\\overline{B} \\to l^+l^+`
        * :math:`B\\overline{B} \\to l^-l^-`
    """
    __authors__ = ["Alessandro Gaz, Chiara La Licata"]
    __contact__ = __liaison__
    __description__ = (
        "Inclusive dilepton skim"
    )
    __category__ = "physics, leptonic"

    NoisyModules = ["EventShapeCalculator"]
    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdMu("all", path=path)

    def build_lists(self, path):
        ma.cutAndCopyList(
            "e+:pid_TDCPV_dilepton",
            "e+:all",
            "abs(dr) < 1 and abs(dz) < 4 and p > 1.2 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu+:pid_TDCPV_dilepton",
            "mu+:all",
            "abs(dr) < 1 and abs(dz) < 4 and p > 1.2 and muonID > 0.5",
            True,
            path=path,
        )

        ma.buildEventShape(
            inputListNames=[],
            default_cleanup=True,
            allMoments=False,
            cleoCones=True,
            collisionAxis=True,
            foxWolfram=True,
            harmonicMoments=True,
            jets=True,
            sphericity=True,
            thrust=True,
            checkForDuplicates=False,
            path=path)

        path = self.skim_event_cuts('sphericity > 0.18 and nTracks > 4', path=path)

        ma.reconstructDecay('Upsilon(4S):ee_TDCPV_dilepton   -> e+:pid_TDCPV_dilepton e-:pid_TDCPV_dilepton', 'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):emu_TDCPV_dilepton  -> e+:pid_TDCPV_dilepton mu-:pid_TDCPV_dilepton', 'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):mumu_TDCPV_dilepton -> mu+:pid_TDCPV_dilepton mu-:pid_TDCPV_dilepton', 'M < 15', path=path)

        ma.reconstructDecay('Delta++:ee_TDCPV_dilepton   -> e+:pid_TDCPV_dilepton e+:pid_TDCPV_dilepton', 'M < 15', path=path)
        ma.reconstructDecay('Delta++:emu_TDCPV_dilepton  -> e+:pid_TDCPV_dilepton mu+:pid_TDCPV_dilepton', 'M < 15', path=path)
        ma.reconstructDecay('Delta++:mumu_TDCPV_dilepton -> mu+:pid_TDCPV_dilepton mu+:pid_TDCPV_dilepton', 'M < 15', path=path)

        ma.copyLists(
            outputListName='Upsilon(4S):ll_TDCPV_dilepton',
            inputListNames=[
                'Upsilon(4S):ee_TDCPV_dilepton',
                'Upsilon(4S):emu_TDCPV_dilepton',
                'Upsilon(4S):mumu_TDCPV_dilepton'],
            path=path)

        ma.copyLists(outputListName='Delta++:ll_TDCPV_dilepton',
                     inputListNames=['Delta++:ee_TDCPV_dilepton', 'Delta++:emu_TDCPV_dilepton', 'Delta++:mumu_TDCPV_dilepton'],
                     path=path)

        return ["Upsilon(4S):ll_TDCPV_dilepton", "Delta++:ll_TDCPV_dilepton"]


@fancy_skim_header
class TDCPV_inclusiveJpsi(BaseSkim):
    """
    **Physics channels**:  bd → ccs

    This skims inclusive J/psi events, which include Psi(2S) to dileptons as well as Psi(2S) to J/psi X or chi_c1 to J/psi gamma

    **Decay Channels**:

    * :math:`B^0 \\to J/\\psi (ee/\\mu\\mu) X`

    **Cuts used**:

    * abs(d0) < 0.5 and abs(z0) < 2.0 and thetaInCDCAcceptance for muon and electron tracks
    * muonID_noSVD > 0.01 for muons and electronID > 0.01 for electrons as PID cuts
    * E < 1 for photons for Bremsstrahlung correction for electrons
    * 2.7 < M < 4.1 and useCMSFrame(p) < 3.1 for J/psi (or Psi(2S)) reconstruction
    """

    __authors__ = ["Xu Dong", "Thibaud Humair", "Tadeas Bilka"]
    __description__ = "Inclusive J/psi (ee/mumu) skim for time-dependent CP violation analysis."
    __contact__ = __liaison__
    __category__ = "physics, TDCPV"

    ApplyHLTHadronCut = True
    validation_sample = _VALIDATION_SAMPLE

    def load_standard_lists(self, path):

        stdE("all", path=path)
        stdMu("all", path=path)
        stdPhotons("loose", path=path)

    def build_lists(self, path):

        # Apply the event-level cuts
        path = self.skim_event_cuts('nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 2.0) >= 3', path=path)

        # NOTE: This is almost like for standad charged list, but without nCDCHits cut,
        # plus we want specific PID cut below, while a typical loose cut is set to 0.1 in stdCharged.py
        goodChargedTrackCut = 'abs(dr) < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance'

        # Fill reduced particle lists for muons, electrons, and photons
        ma.cutAndCopyList('mu+:withPID_TDCPV_inclusiveJpsi', 'mu+:all', f'{goodChargedTrackCut} and muonID_noSVD > 0.01', path=path)
        ma.cutAndCopyList('e+:uncorrected_withPID_TDCPV_inclusiveJpsi', 'e+:all',
                          f'{goodChargedTrackCut} and electronID > 0.01', path=path)
        ma.cutAndCopyList('gamma:bremsinput_TDCPV_inclusiveJpsi', 'gamma:loose', 'E < 1', path=path)

        # Perform bremsstrahlung correction for electrons and fill new list with corrected electrons
        ma.correctBremsBelle(
            'e+:corrected_withPID_TDCPV_inclusiveJpsi',
            'e+:uncorrected_withPID_TDCPV_inclusiveJpsi',
            'gamma:bremsinput_TDCPV_inclusiveJpsi',
            path=path)
        ma.correctBremsBelle('e+:corrected_TDCPV_inclusiveJpsi', 'e+:all', 'gamma:bremsinput_TDCPV_inclusiveJpsi', path=path)

        # Reconstruct J/psi decays
        ma.reconstructDecay(
            'J/psi:inclusive_ee_TDCPV_inclusiveJpsi -> e+:corrected_withPID_TDCPV_inclusiveJpsi e-:corrected_TDCPV_inclusiveJpsi',
            '2.7 < M < 4.1 and useCMSFrame(p) < 3.1',
            path=path)
        ma.reconstructDecay('J/psi:inclusive_mumu_TDCPV_inclusiveJpsi -> mu+:withPID_TDCPV_inclusiveJpsi mu-:all',
                            '2.7 < M < 4.1 and useCMSFrame(p) < 3.1', path=path)

        # Return the particle lists that will be used in the skim
        return ['J/psi:inclusive_ee_TDCPV_inclusiveJpsi', 'J/psi:inclusive_mumu_TDCPV_inclusiveJpsi']

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import ValidationMetadataSetter

        # Define the list of variables to histogram
        variableshisto = [('InvM', 100, 2.7, 4.1)]

        # Define the metadata for the validation histograms
        metadata = [
            ['InvM', 'ee', 'InvM', __liaison__,
             'Invariant mass of the inclusive J/psi in the ee mode', '', 'InvM(e+e-) [GeV]', 'Candidates'],
            ['InvM', 'mumu', 'InvM', __liaison__,
             'Invariant mass of the inclusive J/psi in the mumu mode', '', 'InvM(mu+mu-) [GeV]', 'Candidates']
        ]

        # Define the name of the output file for the validation histograms
        filename = f'{self}_validation.root'

        # Add the metadata to the output file
        path.add_module(ValidationMetadataSetter(metadata, filename))

        # Produce the validation histograms
        ma.variablesToHistogram(
            'J/psi:inclusive_ee_TDCPV_inclusiveJpsi',
            variableshisto,
            filename=filename,
            path=path,
            directory="ee")
        ma.variablesToHistogram(
            'J/psi:inclusive_mumu_TDCPV_inclusiveJpsi',
            variableshisto,
            filename=filename,
            path=path,
            directory="mumu")
