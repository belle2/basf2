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

    * ``B0 -> phi K_S0``
    * ``B0 -> phi K*``
    * ``B0 -> eta K_S0``
    * ``B0 -> eta' K_S0``
    * ``B0 -> eta K*``
    * ``B0 -> eta' K*``
    * ``B0 -> K_S0 K_S0 K_S0``
    * ``B0 -> pi0 K_S0``
    * ``B0 -> rho0 K_S0``
    * ``B0 -> omega K_S0``
    * ``B0 -> f_0 K_S0``
    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0 K_S0 gamma``
    * ``B0 -> phi K_S0 gamma``
    * ``B0 -> eta K_S0 gamma``
    * ``B0 -> rho0 gamma``
    * ``B0 -> omega gamma``
    * ``B0 -> phi gamma``

    * ``B+ -> eta' K+``
    * ``B+ -> phi K+``
    * ``B+ -> phi K*+``
    * ``B+ -> omega K+``
    * ``B+ -> rho0 K+``
    * ``B+ -> K_S0 K_S0 K+``
    * ``B+ -> pi+ pi- K+ gamma``

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

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1``,
    * ``E_ECL_TDCPV < 9``
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
        ma.reconstructDecay('phi:SkimHighEff2 -> pi0:skim pi-:SkimHighEff pi+:SkimHighEff', '0.97 < M < 1.1', path=path)
        ma.reconstructDecay('K_S0:pi0pi0 -> pi0:skim pi0:skim', '0.4 < M < 0.6', path=path)

        ma.cutAndCopyList('pi0:SkimHighEffCut', 'pi0:SkimHighEff', 'M > 0.105 and M < 0.150', path=path)

        ma.reconstructDecay('K*+:kshort_pip -> K_S0:merged pi+:SkimHighEff', '0.74 < M < 1.04', path=path)
        ma.reconstructDecay('K*+:kp_piz -> K+:SkimHighEff pi0:SkimHighEffCut', '0.74 < M < 1.04', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15', 'gamma:all', '1.4<E<4', path=path)
        ma.cutAndCopyList('gamma:ECMS16', 'gamma:all', '1.6<useCMSFrame(E)', path=path)

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
            'pi0:SkimHighEffCut K_S0:merged',
            'rho0:SkimHighEff K_S0:merged',
            'omega:SkimHighEff K_S0:merged',
            'f_0:SkimHighEff K_S0:merged',
            'pi0:skim pi0:skim K_S0:merged',
            'phi:SkimHighEff K_S0:merged pi0:skim',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged gamma:E15',
            'pi0:skim K_S0:merged gamma:E15',
            'pi0:SkimHighEff K_S0:merged gamma:ECMS16',
            'phi:SkimHighEff2 K_S0:merged',
            'phi:SkimHighEff K_S0:pi0pi0',
            'eta\':SkimHighEff K_S0:pi0pi0',
            'phi:SkimHighEff K_S0:merged gamma:E15',
            'eta:SkimHighEff K_S0:merged gamma:E15',
            'rho0:SkimHighEff gamma:E15',
            'omega:SkimHighEff gamma:E15',
            'phi:SkimHighEff gamma:E15'
        ]

        bu_qqs_Channels = [
            'eta\':SkimHighEff K+:SkimHighEff',
            'phi:SkimHighEff K+:SkimHighEff',
            'phi:SkimHighEff K*+:kshort_pip',
            'phi:SkimHighEff K*+:kp_piz',
            'omega:SkimHighEff K+:SkimHighEff',
            'rho0:SkimHighEff K+:SkimHighEff',
            'K_S0:merged K_S0:merged K+:SkimHighEff',
            'pi+:SkimHighEff pi-:SkimHighEff K+:SkimHighEff gamma:E15'
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

        ma.reconstructDecay("B0:etap -> eta':SkimHighEff K_S0:merged", '5.2 < Mbc < 5.3 and abs(deltaE) < 0.3', path=path)

        ma.reconstructDecay("K_10:all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:Kspipig -> K_10:all gamma:E15",
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
        ma.variablesToHistogram('B0:etap', variableshisto, filename=filename, path=path, directory="etap")
        ma.variablesToHistogram('B0:Kspipig', variableshisto, filename=filename, path=path, directory="Kspipig")
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

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``abs(deltaE) < 0.15``
    * ``nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1``,
    * ``E_ECL_TDCPV < 9``
    * ``foxWolframR2<0.6``
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
        loadStdSkimHighEffEta(path=path)  # eta:SkimHighEff1 (gg), eta:SkimHighEff2 (3pi)
        loadStdSkimHighEffRho0(path=path)
        loadStdSkimHighEffOmega(path=path)

        ma.reconstructDecay('phi:SkimHighEff2 -> pi0:skim pi-:SkimHighEff pi+:SkimHighEff', '0.97 < M < 1.1', path=path)

        ma.cutAndCopyList('eta:SkimKlong2', 'eta:SkimHighEff2', '0.5 < M < 0.6', path=path)  # eta->pipipi0 default 0.4 < M < 0.6

        ma.reconstructDecay('eta\':klong_ggpp -> eta:SkimHighEff1 pi+:SkimHighEff pi-:SkimHighEff',
                            '0.8 < M < 1.1', path=path)  # default 0.8 < M < 1.1
        ma.reconstructDecay('eta\':klong_pipipi -> eta:SkimKlong2 pi+:SkimHighEff pi-:SkimHighEff', '0.9 < M < 1', path=path)
        ma.reconstructDecay('eta\':klong_rhogam -> rho0:SkimHighEff gamma:tight', '0.9 < M < 1', path=path)

        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15', 'gamma:all', '1.4<E<4', path=path)
        ma.cutAndCopyList('gamma:ECMS16', 'gamma:all', '1.6<useCMSFrame(E)', path=path)
        ma.cutAndCopyList(
            'K_L0:allecl_qqs',
            'K_L0:allecl',
            '[clusterPulseShapeDiscriminationMVA<0.5] and [clusterE>0.100]',
            path=path)
        ma.cutAndCopyList(
            'K_L0:allklm_qqs',
            'K_L0:allklm',
            '[klmClusterKlId>0.05] and [klmClusterInnermostLayer<=10] and [klmClusterLayers<=10]',
            path=path)
        ma.copyLists('K_L0:eclklm_qqs_0', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # phi(KK)KL
        ma.copyLists('K_L0:eclklm_qqs_1', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # phi(3pi)KL
        ma.copyLists('K_L0:eclklm_qqs_2', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # eta'(eta(gg)pipi)KL
        ma.copyLists('K_L0:eclklm_qqs_3', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # eta'(eta(3pi)pipi)KL
        ma.copyLists('K_L0:eclklm_qqs_4', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # eta'(rho(pipi)gam)KL
        ma.copyLists('K_L0:eclklm_qqs_5', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # omegaKL
        ma.copyLists('K_L0:eclklm_qqs_6', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # rho0KL
        ma.copyLists('K_L0:eclklm_qqs_7', ['K_L0:allecl_qqs', 'K_L0:allklm_qqs'], path=path)  # pi0KL

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV_qqs', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_qqs_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV_qqs', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_qqs_eventshape)')
        vm.addAlias('E_ECL_TDCPV_qqs', 'formula(E_ECL_pi_TDCPV_qqs+E_ECL_gamma_TDCPV_qqs)')

        btotcpvcuts_KL = 'abs(deltaE) < 0.10'

        bd_klong_Channels = [
            'phi:SkimHighEff K_L0:eclklm_qqs_0',
            'phi:SkimHighEff2 K_L0:eclklm_qqs_1',
            'eta\':klong_ggpp K_L0:eclklm_qqs_2',
            'eta\':klong_pipipi K_L0:eclklm_qqs_3',
            'eta\':klong_rhogam K_L0:eclklm_qqs_4',
            'omega:SkimHighEff K_L0:eclklm_qqs_5',
            'rho0:SkimHighEff K_L0:eclklm_qqs_6',
            'pi0:skim K_L0:eclklm_qqs_7'
        ]

        bd_klong_List = []

        for chID, channel in enumerate(bd_klong_Channels):
            ma.reconstructMissingKlongDecayExpert('B0:TDCPV_klong' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path)
            bd_klong_List.append('B0:TDCPV_klong' + str(chID))

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
            "E_ECL_TDCPV_qqs<9",
            "foxWolframR2<0.6"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_klong_List


@fancy_skim_header
class TDCPV_ccs(BaseSkim):
    """
    **Physics channels**:  bd → ccs

    **Decay Channels**:

    * ``B0 -> J/psi (ee/mm) K_S0``
    * ``B0 -> psi(2s) (ee/mm) K_S0``
    * ``B0 -> J/psi (ee/mm) K* (K+ pi- / K_S0 pi0)``
    * ``B+ -> J/psi (ee/mm) K+``
    * ``B0 -> J/psi (ee/mm) KL``
    * ``B0 -> J/psi (ee/mm) eta (pi+ pi- pi0 / pi+ pi-)``
    * ``B0 -> J/psi (ee/mm) pi0``
    * ``B0 -> J/psi (ee/mm) K+ pi-``
    * ``B+ -> J/psi (ee/mm) K*+ (pi+ K_S0 / K+ pi0)``

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

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29 for Ks/K*``
    * ``abs(deltaE) < 0.3 and 5.05 < Mbc < 5.29 for KL``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(thetaInCDCAcceptance and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
    * ``E_ECL_TDCPV < 9``
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

        ma.reconstructDecay('K*0:neutral -> K_S0:merged pi0:eff40_May2020', '0.74 < M < 1.04', path=path)
        ma.reconstructDecay('K*+:kshort_pip -> K_S0:merged pi+:SkimHighEff', '0.74 < M < 1.04', path=path)
        ma.reconstructDecay('K*+:kp_piz -> K+:SkimHighEff pi0:eff40_May2020', '0.74 < M < 1.04', path=path)

        ma.applyCuts('pi0:eff60_May2020', 'InvM < 0.2', path=path)

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
                           'J/psi:ee K*0:neutral',
                           'J/psi:mumu K*0:neutral',
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

        ma.reconstructDecay('B0:jpsiee -> J/psi:ee K_S0:merged', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)
        ma.reconstructDecay('B0:jpsimumu -> J/psi:mumu K_S0:merged', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

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
        ma.variablesToHistogram('B0:jpsiee', variableshisto, filename=filename, path=path, directory="jpsiee")
        ma.variablesToHistogram('B0:jpsimumu', variableshisto, filename=filename, path=path, directory="jpsimumu")

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
            "e+:pid",
            "e+:all",
            "abs(dr) < 1 and abs(dz) < 4 and p > 1.2 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu+:pid",
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

        ma.reconstructDecay('Upsilon(4S):ee   -> e+:pid e-:pid', 'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):emu  -> e+:pid mu-:pid', 'M < 15', path=path)
        ma.reconstructDecay('Upsilon(4S):mumu -> mu+:pid mu-:pid', 'M < 15', path=path)

        ma.reconstructDecay('Delta++:ee   -> e+:pid e+:pid', 'M < 15', path=path)
        ma.reconstructDecay('Delta++:emu  -> e+:pid mu+:pid', 'M < 15', path=path)
        ma.reconstructDecay('Delta++:mumu -> mu+:pid mu+:pid', 'M < 15', path=path)

        ma.copyLists(outputListName='Upsilon(4S):ll',
                     inputListNames=['Upsilon(4S):ee', 'Upsilon(4S):emu', 'Upsilon(4S):mumu'],
                     path=path)

        ma.copyLists(outputListName='Delta++:ll',
                     inputListNames=['Delta++:ee', 'Delta++:emu', 'Delta++:mumu'],
                     path=path)

        return ["Upsilon(4S):ll", "Delta++:ll"]
