#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
                                            loadStdSkimHighEffF_0)

from skim import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons
from stdPi0s import loadStdSkimPi0, stdPi0s
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
    * ``B0 -> pi0 pi0 K_S0``
    * ``B0 -> phi K_S0 pi0``
    * ``B0 -> pi+ pi- K_S0``
    * ``B0 -> pi+ pi- K_S0 gamma``
    * ``B0 -> pi0  K_S0 gamma``
    * ``B+ -> eta' K+``
    * ``B+ -> phi K+``

    **Particle lists used**:

    * ``phi:SkimHighEff``
    * ``eta':SkimHighEff``
    * ``eta:SkimHighEff``
    * ``pi0:eff40_May2020``
    * ``pi0:skim``
    * ``rho0:SkimHighEff``
    * ``omega:SkimHighEff``
    * ``f_0:SkimHighEff``
    * ``pi+:SkimHighEff``
    * ``K+:SkimHighEff``
    * ``omega:SkimHighEff``
    * ``K*0:SkimHighEff``
    * ``gamma:E15 , cut : 1.4 < E < 4``
    * ``k_S0:merged``
    * ``K+:1%``

    **Cuts used**:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3``
    * ``nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1``,
    * ``visibleEnergyOfEventCMS>4"``,
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
        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_May2020", path=path)

        loadStdSkimHighEffPhi(path=path)
        loadStdSkimHighEffEta(path=path)
        loadStdSkimHighEffEtaPrime(path=path)
        loadStdSkimHighEffKstar0(path=path)
        loadStdSkimHighEffRho0(path=path)
        loadStdSkimHighEffOmega(path=path)
        loadStdSkimHighEffF_0(path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('gamma:E15', 'gamma:all', '1.4<E<4', path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_qqs_Channels = [
            'phi:SkimHighEff K_S0:merged',
            'eta\':SkimHighEff K_S0:merged',
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
        ]

        bu_qqs_Channels = [
            'eta\':SkimHighEff K+:SkimHighEff',
            'phi:SkimHighEff K+:SkimHighEff',
        ]

        bd_qqs_List = []
        for chID, channel in enumerate(bd_qqs_Channels):
            ma.reconstructDecay('B0:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TDCPV_qqs' + str(chID), 'nTracks>4', path=path)
            bd_qqs_List.append('B0:TDCPV_qqs' + str(chID))

        bu_qqs_List = []
        for chID, channel in enumerate(bu_qqs_Channels):
            ma.reconstructDecay('B+:TDCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B+:TDCPV_qqs' + str(chID), 'nTracks>4', path=path)
            bu_qqs_List.append('B+:TDCPV_qqs' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
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
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_qqs_List + bu_qqs_List

    def validation_histograms(self, path):
        ma.reconstructDecay("B0:etap -> eta':SkimHighEff K_S0:merged", '5.20 < Mbc < 5.3 and abs(deltaE) < 0.3', path=path)

        Kres = 'K_10'
        ma.applyCuts('gamma:E15', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:Kspipig -> " + Kres + ":all gamma:E15",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        filename = f'{self}_Validation.root'
        ma.variablesToHistogram('B0:etap', variableshisto, filename=filename, path=path, directory="etap")
        ma.variablesToHistogram('B0:Kspipig', variableshisto, filename=filename, path=path, directory="Kspipig")


@fancy_skim_header
class TDCPV_ccs(BaseSkim):
    """
    **Physics channels**:  bd → ccs

    **Decay Channels**:

    * ``B0 -> J/psi (ee/mm) K_S0``
    * ``B0 -> psi(2s) (ee/mm) K_S0``
    * ``B0 -> J/psi (ee/mm) K*``
    * ``B+ -> J/psi (ee/mm) K+``
    * ``B0 -> J/psi (ee/mm) KL``

    **Particle lists used**:

    * ``k_S0:merged``
    * ``pi+:all``
    * ``J/psi:ee``
    * ``J/psi:mumu``
    * ``psi(2S):ee``
    * ``psi(2S):mumu``
    * ``K*0:SkimHighEff``
    * ``K+:SkimHighEff``
    * ``K_L0:all``

    **Cuts used**:

    * ``SkimHighEff tracks thetaInCDCAcceptance AND chiProb > 0 AND abs(dr) < 0.5 AND abs(dz) < 3 and PID>0.01``
    * ``5.2 < Mbc < 5.29 for Ks/K*``
    * ``5.05 < Mbc < 5.29 for KL``
    * ``abs(deltaE) < 0.5``
    * ``nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3``
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

        loadStdSkimHighEffTracks('pi', path=path)
        loadStdSkimHighEffTracks('K', path=path)

        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        stdPi0s("eff40_May2020", path=path)
        loadStdSkimHighEffKstar0(path=path)

        loadStdJpsiToee(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2lepton(path=path)
        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        ma.cutAndCopyList('K_L0:alleclEcut', 'K_L0:allecl', 'E>0.15', path=path)
        ma.copyLists('K_L0:all_klmecl', ['K_L0:allklm', 'K_L0:allecl'], writeOut=True, path=path)

    def build_lists(self, path):
        vm.addAlias('E_ECL_pi_TDCPV', 'totalECLEnergyOfParticlesInList(pi+:TDCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TDCPV', 'totalECLEnergyOfParticlesInList(gamma:TDCPV_eventshape)')
        vm.addAlias('E_ECL_TDCPV', 'formula(E_ECL_pi_TDCPV+E_ECL_gamma_TDCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'
        btotcpvcuts_KL = '5.05 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_ccs_Channels = ['J/psi:ee K_S0:merged',
                           'J/psi:mumu K_S0:merged',
                           'psi(2S):ll K_S0:merged',
                           'J/psi:ee K*0:SkimHighEff',
                           'J/psi:mumu K*0:SkimHighEff']

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
            ma.reconstructDecay('B0:TDCPV_JPsiKL' + str(chID) + ' -> ' + channel, btotcpvcuts_KL, chID, path=path)
            b0toJPsiKL_List.append('B0:TDCPV_JPsiKL' + str(chID))

        ma.fillParticleList(decayString='pi+:TDCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
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
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TDCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        return bd_ccs_List + bPlustoJPsiK_List + b0toJPsiKL_List

    def validation_histograms(self, path):
        ma.reconstructDecay('B0:jpsiee -> J/psi:ee K_S0:merged', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)
        ma.reconstructDecay('B0:jpsimumu -> J/psi:mumu K_S0:merged', '5.24 < Mbc < 5.3 and abs(deltaE) < 0.15', path=path)

        filename = f'{self}_Validation.root'
        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:jpsiee', variableshisto, filename=filename, path=path, directory="jpsiee")
        ma.variablesToHistogram(
            'B0:jpsimumu',
            variableshisto,
            filename=filename,
            path=path,
            directory="jpsimumu")
