#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""""""

import modularAnalysis as ma
from skim.standardlists.charm import (loadKForBtoHadrons, loadPiForBtoHadrons,
                                      loadStdD0_Kpi, loadStdD0_Kpipipi)
from skim.standardlists.dileptons import (loadStdDiLeptons, loadStdJpsiToee,
                                          loadStdJpsiTomumu, loadStdPsi2s2mumu,
                                          loadStdPsi2s2ee)
from skim.standardlists.lightmesons import (loadStdAllPhi, loadStdAllEta,
                                            loadStdAllEtaPrime, loadStdAllRho0,
                                            loadStdAllOmega, loadStdAllF_0,
                                            loadStdPi0ForBToHadrons, loadStdAllKstar0)
from skimExpertFunctions import BaseSkim, fancy_skim_header
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import loadStdSkimPhoton, stdPhotons
from stdPi0s import loadStdSkimPi0, stdPi0s
from stdV0s import stdKshorts
from variables import variables as vm
from stdKlongs import stdKlongs
from vertex import raveFit

__liaison__ = "Chiara La Licata <chiara.lalicata@ts.infn.it>"


@fancy_skim_header
class TCPV(BaseSkim):
    """
    **Physics channels**: bd → qqs and bd → ccs

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

    **Particle lists used**:

    * ``phi:loose``
    * ``k_S0:all``
    * ``eta:loose``
    * ``pi0:eff40_Jan2020``
    * ``pi0:skim``
    * ``rho0:loose``
    * ``pi+:all``
    * ``gamma:E15 , cut : 1.4 < E < 4``
    * ``omega:loose``
    * ``J/psi: eeLoose``
    * ``J/psi: mumuLoose``
    * ``psi(2S): eeLoose``
    * ``psi(2S): mumuloose``
    * ``K*0:loose``
    * ``phi:loose``

    **Cuts used**:

    * ``5.2 < Mbc < 5.29``
    * ``abs(deltaE) < 0.5``
    """

    __authors__ = ["Reem Rasheed", "Chiara La Licata"]
    __description__ = "Skim for time-dependent CP violation analysis."
    __contact__ = __liaison__
    __category__ = "physics, TCPV"

    ApplyHLTHadronCut = True

    def load_standard_lists(self, path):
        stdE("all", path=path)
        stdK("all", path=path)
        stdMu("all", path=path)
        stdPi("all", path=path)
        stdPhotons("all", path=path)
        stdPi0s("eff40_Jan2020", path=path)
        loadStdSkimPi0(path=path)
        stdKshorts(path=path)
        loadStdJpsiToee(path=path)
        loadStdJpsiTomumu(path=path)
        loadStdPsi2s2mumu(path=path)
        loadStdPsi2s2ee(path=path)
        loadStdAllPhi(path=path)
        loadStdAllEta(path=path)
        loadStdAllEtaPrime(path=path)
        loadStdAllRho0(path=path)
        loadStdAllOmega(path=path)
        loadStdAllF_0(path=path)
        loadStdAllKstar0(path=path)
        stdKlongs(listtype='allklm', path=path)
        stdKlongs(listtype='allecl', path=path)

    def additional_setup(self, path):
        Kcut = "dr < 0.5 and abs(dz) < 2 and thetaInCDCAcceptance and kaonID > 0.01"
        ma.fillParticleList('K+:1%', cut=Kcut, path=path)
        ma.cutAndCopyList('gamma:E15', 'gamma:all', '1.4<E<4', path=path)
        ma.cutAndCopyList('K_L0:alleclEcut', 'K_L0:allecl', 'E>0.15', path=path)
        ma.copyLists('K_L0:all', ['K_L0:allklm', 'K_L0:allecl'], writeOut=True, path=path)

    def build_lists(self, path):
        vm.addAlias('foxWolframR2_maskedNaN', 'ifNANgiveX(foxWolframR2,1)')
        vm.addAlias('E_ECL_pi_TCPV', 'totalECLEnergyOfParticlesInList(pi+:TCPV_eventshape)')
        vm.addAlias('E_ECL_gamma_TCPV', 'totalECLEnergyOfParticlesInList(gamma:TCPV_eventshape)')
        vm.addAlias('E_ECL_TCPV', 'formula(E_ECL_pi_TCPV+E_ECL_gamma_TCPV)')

        btotcpvcuts = '5.2 < Mbc < 5.29 and abs(deltaE) < 0.5'
        btotcpvcuts_KL = '5.05 < Mbc < 5.29 and abs(deltaE) < 0.5'

        bd_qqs_Channels = [
            'phi:all K_S0:merged',
            'eta\':all K_S0:merged',
            'eta:all K_S0:merged',
            'eta\':all K*0:all',
            'eta:all K*0:all',
            'K_S0:merged K_S0:merged K_S0:merged',
            'pi0:skim K_S0:merged',
            'rho0:all K_S0:merged',
            'omega:all K_S0:merged',
            'f_0:all K_S0:merged',
            'pi0:skim pi0:skim K_S0:merged',
            'phi:all K_S0:merged pi0:skim',
            'pi+:all pi-:all K_S0:merged',
            'pi+:all pi-:all K_S0:merged gamma:E15',
            'pi0:skim K_S0:merged gamma:E15',
        ]

        bd_ccs_Channels = ['J/psi:ee K_S0:merged',
                           'J/psi:mumu K_S0:merged',
                           'psi(2S):ee K_S0:merged',
                           'psi(2S):mumu K_S0:merged',
                           'J/psi:ee K*0:all',
                           'J/psi:mumu K*0:all']

        bPlustoJPsiK_Channel = ['J/psi:mumu K+:1%',
                                'J/psi:ee K+:1%']

        bd_qqs_List = []
        for chID, channel in enumerate(bd_qqs_Channels):
            ma.reconstructDecay('B0:TCPV_qqs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TCPV_qqs' + str(chID), 'nTracks>4', path=path)
            bd_qqs_List.append('B0:TCPV_qqs' + str(chID))

        bd_ccs_List = []
        for chID, channel in enumerate(bd_ccs_Channels):
            ma.reconstructDecay('B0:TCPV_ccs' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            ma.applyCuts('B0:TCPV_ccs' + str(chID), 'nTracks>4', path=path)
            bd_ccs_List.append('B0:TCPV_ccs' + str(chID))

        bPlustoJPsiK_List = []

        for chID, channel in enumerate(bPlustoJPsiK_Channel):
            ma.reconstructDecay('B+:TCPV_JPsiK' + str(chID) + ' -> ' + channel, btotcpvcuts, chID, path=path)
            bPlustoJPsiK_List.append('B+:TCPV_JPsiK' + str(chID))

        b0toJPsiKL_List = []
        ma.reconstructMissingKlongDecayExpert('B0:TCPV_JPsiKL_mm -> J/psi:mumu K_L0:all',
                                              btotcpvcuts_KL, 1, True, path=path, recoList='_reco1')
        ma.reconstructMissingKlongDecayExpert('B0:TCPV_JPsiKL_ee -> J/psi:ee K_L0:all',
                                              btotcpvcuts_KL, 1, True, path=path, recoList='_reco2')
        b0toJPsiKL_List.append('B0:TCPV_JPsiKL_mm')
        b0toJPsiKL_List.append('B0:TCPV_JPsiKL_ee')

        ma.fillParticleList(decayString='pi+:TCPV_eventshape',
                            cut='pt > 0.1 and abs(d0)<0.5 and abs(z0)<2 and nCDCHits>20', path=path)
        ma.fillParticleList(decayString='gamma:TCPV_eventshape',
                            cut='E > 0.1 and 0.296706 < theta < 2.61799', path=path)

        ma.buildEventShape(inputListNames=['pi+:TCPV_eventshape', 'gamma:TCPV_eventshape'],
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

        ma.buildEventKinematics(inputListNames=['pi+:TCPV_eventshape', 'gamma:TCPV_eventshape'], path=path)

        EventCuts = [
            "foxWolframR2_maskedNaN<0.4 and nTracks>=4",
            "nCleanedTracks(abs(z0) < 2.0 and abs(d0) < 0.5 and nCDCHits>20)>=3",
            "nCleanedECLClusters(0.296706 < theta < 2.61799 and E>0.2)>1",
            "visibleEnergyOfEventCMS>4",
            "E_ECL_TCPV<9"
        ]
        path = self.skim_event_cuts(" and ".join(EventCuts), path=path)

        tcpvLists = bd_qqs_List + bd_ccs_List + bPlustoJPsiK_List + b0toJPsiKL_List

        self.SkimLists = tcpvLists

    def validation_histograms(self, path):
        Kres = 'K_10'
        ma.applyCuts('gamma:loose', '1.4 < E < 4', path=path)

        ma.reconstructDecay(Kres + ":all -> K_S0:merged pi+:all pi-:all ", "", path=path)
        ma.reconstructDecay("B0:signal -> " + Kres + ":all gamma:loose",
                            "Mbc > 5.2 and deltaE < 0.5 and deltaE > -0.5", path=path)
        ma.matchMCTruth('B0:signal', path=path)

        variableshisto = [('deltaE', 100, -0.5, 0.5), ('Mbc', 100, 5.2, 5.3)]
        ma.variablesToHistogram('B0:signal', variableshisto, filename='TCPV_Validation.root', path=path)
