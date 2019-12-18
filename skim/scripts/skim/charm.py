#!/usr/bin/env python3
# -*- coding: utf-8 -*-

""" Skim list building functions for charm analyses.. """


__authors__ = [
    ""
]

import modularAnalysis as ma
import vertex

haveRunD0ToHpJm = 0
haveRunD0ToNeutrals = 0


def D0ToHpJm(path):
    mySel = 'abs(d0) < 1 and abs(z0) < 3'
    mySel += ' and 0.296706 < theta < 2.61799'
    ma.fillParticleList('pi+:mygood', mySel, path=path)
    ma.fillParticleList('K+:mygood', mySel, path=path)

    charmcuts = '1.80 < M < 1.93 and useCMSFrame(p)>2.2'
    D0_Channels = ['pi+:mygood K-:mygood',
                   'pi+:mygood pi-:mygood',
                   'K+:mygood K-:mygood',
                   ]

    D0List = []

    global haveRunD0ToHpJm
    if haveRunD0ToHpJm == 0:
        haveRunD0ToHpJm = 1
        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay('D0:HpJm' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
            D0List.append('D0:HpJm' + str(chID))
    else:
        for chID, channel in enumerate(D0_Channels):
            D0List.append('D0:HpJm' + str(chID))

    Lists = D0List
    return Lists


def DstToD0PiD0ToHpJm(path):

    D0List = D0ToHpJm(path)

    Dstcuts = '0 < Q < 0.018'

    DstList = []
    for chID, channel in enumerate(D0List):
        ma.reconstructDecay('D*+:HpJm' + str(chID) + ' -> D0:HpJm' + str(chID) + ' pi+:mygood', Dstcuts, chID, path=path)
        DstList.append('D*+:HpJm' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmPi0(path):
    Dstcuts = 'massDifference(0) < 0.160 and useCMSFrame(p) > 2.0'
    charmcuts = '1.70 < M < 2.10'
    ma.cutAndCopyList('pi0:myskim', 'pi0:skim', '', path=path)  # additional cuts removed 27 Jun 2019 by Emma Oxford

    DstList = []
    ma.reconstructDecay('D0:HpJmPi0 -> K-:loose pi+:loose pi0:myskim', charmcuts, path=path)
    ma.reconstructDecay('D*+:HpJmPi0RS -> D0:HpJmPi0 pi+:all', Dstcuts, path=path)
    ma.reconstructDecay('D*-:HpJmPi0WS -> D0:HpJmPi0 pi-:all', Dstcuts, path=path)
    ma.copyLists('D*+:HpJmPi0', ['D*+:HpJmPi0RS', 'D*+:HpJmPi0WS'], path=path)
    DstList.append('D*+:HpJmPi0')

    return DstList


def EarlyData_DstToD0PiD0ToHpJmPi0(path):
    mySel = 'abs(d0) < 0.5 and abs(z0) < 1.0'  # IP cut, tighter than previous skims
    mySel += ' and 0.296706 < theta < 2.61799'  # CDC acceptance cut
    ma.fillParticleList('pi+:myhjp0', mySel, path=path)
    ma.fillParticleList('K+:myhjp0', mySel, path=path)

    ma.cutAndCopyList('pi0:myhjp0', 'pi0:skim', '', path=path)  # see analysis/scripts/stdPi0s.py for cuts

    D0cuts = '1.70 < M < 2.10'
    Dstcuts = 'massDifference(0) < 0.160 and useCMSFrame(p) > 2.0'

    eventcuts = 'nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 1.0) >= 3'
    ma.applyEventCuts(eventcuts, path=path)

    DstList = []
    ma.reconstructDecay('D0:HpJmPi0 -> K-:myhjp0 pi+:myhjp0 pi0:myhjp0', D0cuts, path=path)
    ma.reconstructDecay('D*+:HpJmPi0RS -> D0:HpJmPi0 pi+:myhjp0', Dstcuts, path=path)
    ma.reconstructDecay('D*-:HpJmPi0WS -> D0:HpJmPi0 pi-:myhjp0', Dstcuts, path=path)
    ma.copyLists('D*+:HpJmPi0', ['D*+:HpJmPi0RS', 'D*+:HpJmPi0WS'], path=path)
    DstList.append('D*+:HpJmPi0')

    return DstList


def DstToD0PiD0ToHpHmPi0(path):
    Dstcuts = 'massDifference(0) < 0.160 and useCMSFrame(p) > 2.0'
    charmcuts = '1.70 < M < 2.10'
    ma.cutAndCopyList('pi0:myskim', 'pi0:skim', '', path=path)  # additional cuts removed 27 Jun 2019 by Emma Oxford
    D0_Channels = ['pi+:loose pi-:loose pi0:myskim',
                   'K+:loose K-:loose pi0:myskim',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay('D0:HpHmPi0' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        ma.reconstructDecay('D*+:HpHmPi0' + str(chID) + ' -> D0:HpHmPi0' + str(chID) + ' pi+:all', Dstcuts, chID, path=path)
        DstList.append('D*+:HpHmPi0' + str(chID))

    return DstList


def EarlyData_DstToD0PiD0ToHpHmPi0(path):
    mySel = 'abs(d0) < 0.5 and abs(z0) < 1.0'  # IP cut, tighter than previous skims
    mySel += ' and 0.296706 < theta < 2.61799'  # CDC acceptance cut
    ma.fillParticleList('pi+:myhhp0', mySel, path=path)
    ma.fillParticleList('K+:myhhp0', mySel, path=path)

    ma.cutAndCopyList('pi0:myhhp0', 'pi0:skim', '', path=path)  # see analysis/scripts/stdPi0s.py for cuts

    D0cuts = '1.70 < M < 2.10'
    Dstcuts = 'massDifference(0) < 0.160 and useCMSFrame(p) > 2.0'

    eventcuts = 'nCleanedTracks(abs(d0) < 0.5 and abs(z0) < 1.0) >= 3'
    ma.applyEventCuts(eventcuts, path=path)

    D0_Channels = ['pi+:myhhp0 pi-:myhhp0 pi0:myhhp0',
                   'K+:myhhp0 K-:myhhp0 pi0:myhhp0',
                   ]

    DstList = []

    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay('D0:HpHmPi0' + str(chID) + ' -> ' + channel, D0cuts, chID, path=path)
        ma.reconstructDecay('D*+:HpHmPi0' + str(chID) + ' -> D0:HpHmPi0' + str(chID) + ' pi+:myhhp0', Dstcuts, chID, path=path)
        DstList.append('D*+:HpHmPi0' + str(chID))

    return DstList


def DstToD0PiD0ToHpJmEta(path):
    ma.reconstructDecay('eta:myskim -> gamma:loose gamma:loose', '0.49 < M < 0.55 and p > 0.28', path=path)
    Dstcuts = '0 < Q < 0.018'
    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p) > 2.2'

    DstList = []
    ma.reconstructDecay('D0:HpJmEta -> K-:loose pi+:loose eta:myskim', charmcuts, path=path)
    vertex.vertexTree('D0:HpJmEta', 0.001, path=path)
    ma.reconstructDecay('D*+:HpJmEtaRS -> D0:HpJmEta pi+:all', Dstcuts, path=path)
    ma.reconstructDecay('D*-:HpJmEtaWS -> D0:HpJmEta pi-:all', Dstcuts, path=path)
    vertex.vertexKFit('D*+:HpJmEtaRS', conf_level=0.001, path=path)
    vertex.vertexKFit('D*+:HpJmEtaWS', conf_level=0.001, path=path)
    DstList.append('D*+:HpJmEtaRS')
    DstList.append('D*+:HpJmEtaWS')

    return DstList


def DstToD0PiD0ToKsOmega(path):
    ma.cutAndCopyList('pi0:mypi0', 'pi0:skim', '0.11 < M < 0.15 and p > 0.25 ', path=path)
    ma.reconstructDecay('eta:3pi -> pi+:loose pi-:loose pi0:mypi0', '0.4 < M < 0.65', path=path)
    ma.reconstructDecay('omega:3pi -> pi+:loose pi-:loose pi0:mypi0', '0.65 < M < 0.9', path=path)

    charmcuts = '1.78 < M < 1.93 and useCMSFrame(p)>2.2'
    ma.reconstructDecay('D0:Eta -> K_S0:merged eta:3pi', charmcuts, path=path)
    vertex.vertexTree('D0:Eta', conf_level=0.001, path=path)
    ma.reconstructDecay('D0:Omega -> K_S0:merged omega:3pi', charmcuts, path=path)
    vertex.vertexTree('D0:Omega', conf_level=0.001, path=path)
    ma.copyLists('D0:KsOmega', ['D0:Eta', 'D0:Omega'], path=path)

    DstList = []
    ma.reconstructDecay('D*+:KsOmega -> D0:KsOmega pi+:all', '0 < Q < 0.018', path=path)
    vertex.vertexKFit('D*+:KsOmega', conf_level=0.001, path=path)
    DstList.append('D*+:KsOmega')

    return DstList


def D0ToNeutrals(path):
    charmcuts = '1.78 < M < 1.94 and useCMSFrame(p)>2.2'
    D0_Channels = ['pi0:skim pi0:skim',
                   'K_S0:merged pi0:skim',
                   'K_S0:merged K_S0:merged',
                   ]

    D0List = []

    global haveRunD0ToNeutrals
    if haveRunD0ToNeutrals == 0:
        haveRunD0ToNeutrals = 1
        for chID, channel in enumerate(D0_Channels):
            ma.reconstructDecay('D0:2Nbdy' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
            D0List.append('D0:2Nbdy' + str(chID))
    else:
        for chID, channel in enumerate(D0_Channels):
            D0List.append('D0:2Nbdy' + str(chID))

    Lists = D0List
    return Lists


def DstToD0Neutrals(path):

    D0List = D0ToNeutrals(path)

    Dstcuts = '0 < Q < 0.02'

    DstList = []
    for chID, channel in enumerate(D0List):
        ma.reconstructDecay('D*+:2Nbdy' + str(chID) + ' -> pi+:all ' + channel, Dstcuts, chID, path=path)
        DstList.append('D*+:2Nbdy' + str(chID))

    return DstList


def DstToD0PiD0ToHpHmKs(path):

    D0cuts = '1.80 < M < 1.93'
    Dstcuts = '0 < Q < 0.015 and useCMSFrame(p)>2.3'

    D0_Channels = ['pi-:loose pi+:loose K_S0:merged',
                   'K-:loose K+:loose K_S0:merged'
                   ]
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay('D0:HpHmKs' + str(chID) + ' -> ' + channel, D0cuts, chID, path=path)

        ma.reconstructDecay('D*+:HpHmKs' + str(chID) + ' -> pi+:all D0:HpHmKs' + str(chID), Dstcuts, chID, path=path)
        DstList.append('D*+:HpHmKs' + str(chID))

    return DstList


def CharmRare(path):
    charmcuts = '1.78 < M < 1.94'
    Dstcuts = '0 < Q < 0.02 and 2.2 < useCMSFrame(p)'

    D0_Channels = ['gamma:skim gamma:skim',
                   'e+:loose e-:loose',
                   'e+:loose mu-:loose',
                   'e-:loose mu+:loose',
                   'mu+:loose mu-:loose',
                   'pi+:loose pi-:loose']
    DstList = []

    for chID, channel in enumerate(D0_Channels):
        ma.reconstructDecay('D0:Rare' + str(chID) + ' -> ' + channel, charmcuts, chID, path=path)
        ma.reconstructDecay('D*+:' + str(chID) + ' -> pi+:loose D0:Rare' + str(chID),
                            Dstcuts, chID, path=path)
        DstList.append('D*+:' + str(chID))

    return DstList


def CharmSemileptonic(path):
    Dcuts = '1.82 < M < 1.92'
    DstarSLcuts = '1.9 < M < 2.1'
    antiD0SLcuts = 'massDifference(0)<0.15'

    D_Channels = ['K-:95eff pi+:95eff',
                  'K-:95eff pi+:95eff pi0:skim',
                  'K-:95eff pi+:95eff pi+:95eff pi-:95eff',
                  'K-:95eff pi+:95eff pi+:95eff pi-:95eff pi0:skim',
                  ]

    DList = []
    for chID, channel in enumerate(D_Channels):
        ma.reconstructDecay('D0:std' + str(chID) + ' -> ' + channel, Dcuts, chID, path=path)
        DList.append('D0:std' + str(chID))
    ma.copyLists('D0:all', DList, path=path)

    DstarSLRecoilChannels = ['D0:all pi+:95eff',
                             ]

    antiD0List = []
    for chID, channel in enumerate(DstarSLRecoilChannels):
        ma.reconstructRecoil(decayString='D*-:SL' + str(chID) + ' -> ' + channel,
                             cut=DstarSLcuts, dmID=chID, path=path)
        ma.reconstructRecoilDaughter(decayString='anti-D0:SL' + str(chID) + ' -> D*-:SL' + str(chID) +
                                     ' pi-:95eff', cut=antiD0SLcuts, dmID=chID, path=path)
        antiD0List.append('anti-D0:SL' + str(chID))

    nueRecoilChannels = []
    for channel in antiD0List:
        # nueRecoilChannels.append(channel + ' K+:95eff e-:std')
        # nueRecoilChannels.append(channel + ' pi+:95eff e-:std')
        nueRecoilChannels.append(channel + ' K+:95eff e-:95eff')
        nueRecoilChannels.append(channel + ' pi+:95eff e-:95eff')

    numuRecoilChannels = []
    for channel in antiD0List:
        # numuRecoilChannels.append(channel + ' K+:95eff mu-:std')
        # numuRecoilChannels.append(channel + ' pi+:95eff mu-:std')
        numuRecoilChannels.append(channel + ' K+:95eff mu-:95eff')
        numuRecoilChannels.append(channel + ' pi+:95eff mu-:95eff')

    nueList = []
    for chID, channel in enumerate(nueRecoilChannels):
        ma.reconstructRecoilDaughter(decayString='anti-nu_e:SL' + str(chID) + ' -> ' + channel,
                                     cut='', dmID=chID, path=path)
        nueList.append('anti-nu_e:SL' + str(chID))

    numuList = []
    for chID, channel in enumerate(numuRecoilChannels):
        ma.reconstructRecoilDaughter(decayString='anti-nu_mu:SL' + str(chID) + ' -> ' + channel,
                                     cut='', dmID=chID, path=path)
        numuList.append('anti-nu_mu:SL' + str(chID))

    allLists = nueList + numuList
    return allLists


def DpToKsHp(path):
    Dpcuts = '1.72 < M < 1.98 and useCMSFrame(p)>2'
    Dp_Channels = ['K_S0:merged pi+:loose',
                   'K_S0:merged K+:loose',
                   ]

    DpList = []
    for chID, channel in enumerate(Dp_Channels):
        ma.reconstructDecay('D+:KsHp' + str(chID) + ' -> ' + channel, Dpcuts, chID, path=path)
        DpList.append('D+:KsHp' + str(chID))

    Lists = DpList
    return Lists
