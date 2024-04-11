#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import modularAnalysis as ma
from skim.standardlists.dileptons import (loadStdJpsiToee_noTOP, loadStdJpsiTomumu,
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

# __liaison__ = "Chiara La Licata <chiara.lalicata@ts.infn.it>"
__liaison__ = "Yoshiyuki ONUKI <onuki@hep.phys.s.u-tokyo.ac.jp>"
_VALIDATION_SAMPLE = "mdst14.root"


@fancy_skim_header
class TDCPV_qqs(BaseSkim):
    """
    **Physics channels**: bd/u → qqs

    **Decay Channels**:

    * ``B0 -> phi K_S0``
    * ``B0 -> eta K_S0``
    * ``B0 -> eta' K_S0``
    * ``B0 -> eta' K_L0``
    * ``B0 -> eta K*``
    * ``B0 -> eta' K*``
    * ``B0 -> K_S0 K_S0 K_S0``
    * ``B0 -> pi0 K_S0``
    * ``B0 -> rho0 K_S0``
    * ``B0 -> omega  K_S0``
    * ``B0 -> f_0 K_S0``
    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0  K_S0 gamma``
    * ``B+ -> eta' K+``
    * ``B+ -> phi K+``
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
    * ``K_L0:allklm``
    * ``K_L0:allecl``

    **Cuts used**:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    * ``abs(deltaE) < 0.250 for KL``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``E_ECL_TDCPV<9``
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

        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15', 'gamma:all', '1.4<E<4', path=path)
        ma.cutAndCopyList('gamma:ECMS16', 'gamma:all', '1.6<useCMSFrame(E)', path=path)
        ma.cutAndCopyList('K_L0:eclEcut', 'K_L0:allecl', 'E>0.250', path=path)
        ma.cutAndCopyList('K_L0:klmLayers', 'K_L0:allklm', '[klmClusterInnermostLayer<=10] and [klmClusterLayers<=10]', path=path)
        ma.copyLists('K_L0:klmecl', ['K_L0:klmLayers', 'K_L0:eclEcut'], path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'
        btotcpvcuts_KL = 'abs(deltaE) < 0.250'

        bd_qqs_Channels = [
            'phi:SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K_L0:eclEcut',
            'eta:SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K*0:SkimHighEff',
            'eta:SkimHighEff K*0:SkimHighEff',
            'K_S0:merged K_S0:merged K_S0:merged',
            'pi0:skim K_S0:merged',
            'rho0:SkimHighEff K_S0:merged',
            'omega:SkimHighEff K_S0:merged',
            'f_0:SkimHighEff K_S0:merged',
            'pi0:skim pi0:skim K_S0:merged',
            'phi:SkimHighEff K_S0:merged pi0:skim',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged',
            'pi+:SkimHighEff pi-:SkimHighEff K_S0:merged gamma:E15',
            'pi0:skim K_S0:merged gamma:E15',
            'pi0:SkimHighEff K_S0:merged gamma:ECMS16',
        ]

        bd_qqs_KL_Channels = ['eta\':SkimHighEff  K_L0:klmecl']

        bu_qqs_Channels = [
            'eta\':SkimHighEff K+:SkimHighEff',
            'phi:SkimHighEff K+:SkimHighEff',
            'pi+:SkimHighEff pi-:SkimHighEff K+:SkimHighEff gamma:E15',
        ]

        bd_qqs_List = []
        for chID, channel in enumerate(bd_qqs_Channels):
            ma.reconstructDecay('B0:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bd_qqs_List.append('B0:TDCPV_qqs' + str(chID))

        bd_qqs_KL_List = []
        for chID, channel in enumerate(bd_qqs_KL_Channels):
            ma.reconstructMissingKlongDecayExpert('B0:TDCPV_qqs_KL' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path)
            bd_qqs_KL_List.append('B0:TDCPV_qqs_KL' + str(chID))

        bu_qqs_List = []
        for chID, channel in enumerate(bu_qqs_Channels):
            ma.reconstructDecay('B+:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bu_qqs_List.append('B+:TDCPV_qqs' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(dr)<0.5 and abs(dz)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'], path=path)

        EventCuts = [
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_qqs_List + bu_qqs_List + bd_qqs_KL_List

    def validation_histograms(self, path):
        # NOTE: the validation package is not part of the light releases, so this import
        # must be made here rather than at the top of the file.
        from validation_tools.metadata import ValidationMetadataSetter

        ma.reconstructDecay("B0:etap -> eta':SkimHighEff K_S0:merged", '5.20 < Mbc < 5.3 and abs(deltaE) < 0.3', path=path)

        Kres = 'K_10'
        ma.applyCuts('gamma:E15', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:Kspipig -> " + Kres + ":all gamma:E15",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        filename = f'{self}_Validation.root'
        metadata = []
        for directory in ["etap", "Kspipig"]:
            metadata.append(['deltaE', directory, '#Delta E', __liaison__,
                            f'Energy difference of B for {directory} mode', '', '#Delta E [GeV]', 'Candidates'])
            metadata.append(['Mbc', directory, 'Mbc', __liaison__,
                            f'Beam-constrained mass for {directory} mode', '', 'M_{bc} [GeV]', 'Candidates'])
        metadata.append(['deltaE', 'KL_etap', '#Delta E', __liaison__,
                         "Energy difference of B for B0 -> eta' K_{L}", '', '#Delta E [GeV]', 'Candidates'])
        path.add_module(ValidationMetadataSetter(metadata, filename))
        ma.variablesToHistogram('B0:etap', variableshisto, filename=filename, path=path, directory="etap")
        ma.variablesToHistogram('B0:Kspipig', variableshisto, filename=filename, path=path, directory="Kspipig")
        variableshisto = [('deltaE', 135, -0.020, 0.250)]
        ma.variablesToHistogram('B0:TDCPV_qqs_KL0', variableshisto, filename=filename, path=path, directory="KL_etap")


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

    **Particle lists used**:

    * ``k_S0:merged``
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
    * ``abs(deltaE) < 0.3 for KL``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
    * ``E_ECL_TDCPV<9``
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

        loadStdJpsiToee_noTOP(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2lepton(path=path)
        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

        ma.reconstructDecay('K*0:neutral -> K_S0:merged pi0:eff40_May2020', '0.74 < M < 1.04', path=path)
        ma.applyCuts('pi0:eff60_May2020', 'InvM < 0.2', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('K_L0:alleclEcut', 'K_L0:allecl', 'E>0.15', path=path)
        ma.copyLists('K_L0:all_klmecl', ['K_L0:allklm', 'K_L0:alleclEcut'], writeOut=True, path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'
        btotcpvcuts_KL = 'abs(deltaE) < 0.3'

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

        bd_ccs_KL_Channels = ['J/psi:mumu K_L0:all_klmecl',
                              'J/psi:ee K_L0:all_klmecl']

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

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(dr)<0.5 and abs(dz)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TDCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TDCPV_eventshape', 'gamma:TDCPV_eventshape'], path=path)

        EventCuts = [
            "nCleanedTracks(abs(dz) < 2.0 and abs(dr) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
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
            "abs(d0) < 1 and abs(z0) < 4 and p > 1.2 and electronID > 0.5",
            True,
            path=path,
        )
        ma.cutAndCopyList(
            "mu+:pid",
            "mu+:all",
            "abs(d0) < 1 and abs(z0) < 4 and p > 1.2 and muonID > 0.5",
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
