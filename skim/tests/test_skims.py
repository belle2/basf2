#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from ROOT import Belle2
from modularAnalysis import *

from stdCharged import stdPi, stdK, stdE, stdMu, stdPr
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
set_log_level(LogLevel.INFO)

from skimExpertFunctions import setSkimLogging, add_skim

inputMdstList('MC9', Belle2.FileSystem.findFile('analysis/tests/mdst.root'))


stdPi0s('loose')
stdPhotons('loose')
stdKshorts()
stdPi('loose')
stdK('loose')
stdPr('loose')
stdE('loose')
stdMu('loose')
stdPi('all')
stdK('all')
stdE('all')
stdMu('all')

loadStdLightMesons()
loadStdSkimPi0()
loadStdSkimPhoton()
stdPhotons('all')
stdPhotons('tight')  # also builds loose list
stdK('95eff')
stdPi('95eff')
stdE('95eff')
stdMu('95eff')
stdPr('90eff')

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()
loadStdDiLeptons(True)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4')


# ISR cc skim
from skim.quarkonium import ISRpipiccList
add_skim('ISRpipicc', ISRpipiccList())

# BtoPi0Pi0 Skim
from skim.btocharmless import BtoPi0Pi0List
add_skim('BtoPi0Pi0', BtoPi0Pi0List())

# Tau Skim
from skim.taupair import TauLFVList
add_skim('TauLFV', TauLFVList())

from skim.taupair import TauList
# add_skim('TauGeneric', TauList())


# TCPV Skim
from skim.tcpv import TCPVList
add_skim('TCPV', TCPVList())

# Bottomonium Etab Skim: 15420100
from skim.quarkonium import *
add_skim('BottomoniumEtabExclusive', EtabList())

from skim.quarkonium import *
add_skim('BottomoniumUpsilon', UpsilonList())


from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List() + CharmlessHad2BodyBmList()
add_skim('CharmlessHad2Body', Had2BodyList)

from skim.btocharmless import CharmlessHad3BodyB0List, CharmlessHad3BodyBmList

# charm Skim: D0 -> K pi/ pi pi /K K
from skim.charm import D0ToHpJm
add_skim('Charm2BodyHadronicD0', D0ToHpJm())

# charm Skim: D* -> D0 (-> K pi/ pi pi /K K) pi
from skim.charm import DstToD0PiD0ToHpJm
add_skim('Charm2BodyHadronic', DstToD0PiD0ToHpJm())

# charm Skim: D* -> D0 (-> K pi pi0) pi
from skim.charm import DstToD0PiD0ToHpJmPi0
add_skim('Charm3BodyHadronic', DstToD0PiD0ToHpJmPi0())

# charm Skim: D* -> D0 (-> pi pi pi0/K K pi0) pi
from skim.charm import DstToD0PiD0ToHpHmPi0
add_skim('Charm3BodyHadronic1', DstToD0PiD0ToHpHmPi0())

# charm Skim: D* -> D0 (-> K pi eta) pi
from skim.charm import DstToD0PiD0ToHpJmEta
add_skim('Charm3BodyHadronic3', DstToD0PiD0ToHpJmEta())

# charm Skim: D*+ -> D0 (-> Ks omega/eta (-> pi pi pi0)) pi
from skim.charm import DstToD0PiD0ToKsOmega
add_skim('Charm2BodyNeutrals2', DstToD0PiD0ToKsOmega())

# charm Skim: D0 -> pi0 pi0/Ks pi0/Ks Ks
from skim.charm import D0ToNeutrals
add_skim('Charm2BodyNeutralsD0', D0ToNeutrals())

# charm Skim: D* -> D0 (-> pi0 pi0/Ks pi0/Ks Ks) pi
from skim.charm import DstToD0Neutrals
add_skim('Charm2BodyNeutrals', DstToD0Neutrals())

# charm Skim: D* -> D0 (-> pi pi Ks/ K K Ks) pi
from skim.charm import DstToD0PiD0ToHpHmKs
add_skim('Charm3BodyHadronic2', DstToD0PiD0ToHpHmKs())

# charm Skim: D -> gamma gamma/ e e/ mu mu
from skim.charm import CharmRareList
add_skim('CharmRare', CharmRareList())

from skim.charm import CharmSemileptonicList
add_skim('CharmSemileptonic', CharmSemileptonicList())


# Systematics skim
from skim.systematics import SystematicsList
add_skim('Systematics', SystematicsList(analysis_main))

# Systematics Lambda Skim
from skim.systematics import SystematicsLambdaList
add_skim('SystematicsLambda', SystematicsLambdaList(analysis_main))

# Systematics Tracking
from skim.systematics import SystematicsTrackingList
add_skim('SystematicsTracking', SystematicsTrackingList(analysis_main))

# Resonan ce
from skim.systematics import ResonanceList
add_skim('Resonance', ResonanceList(analysis_main))

# Systematics Rad mu mu
from skim.systematics import SystematicsRadMuMuList
add_skim('SystematicsRadMuMu', SystematicsRadMuMuList(analysis_main))

# Systematics Rad mu mu
from skim.systematics import SystematicsRadEEList
add_skim('SystematicsRadEE', SystematicsRadEEList(analysis_main))


from skim.btocharm import BsigToDhTohhList, loadD0bar
loadD0bar()
add_skim('BtoDh_hh', BsigToDhTohhList())


# B- to D(->Kshh)h- Skim
from skim.btocharm import BsigToDhToKshhList, loadDkshh
loadDkshh()
BtoDhKshhList = BsigToDhToKshhList()
add_skim('BtoDh_Kshh', BtoDhKshhList)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import BsigToDhToKspi0List, loadDkspi0
loadDkspi0()
BtoDhKspi0List = BsigToDhToKspi0List()
add_skim('BtoDh_Kspi0', BtoDhKspi0List)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import BsigToDhToKspipipi0List, loadDkspipipi0
loadDkspipipi0()
BtoDhKspipipi0List = BsigToDhToKspipipi0List()
add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List)


from skim.ewp import B2XllList
add_skim('BtoXll', B2XllList())


# EWP Skim
from skim.ewp import B2XgammaList
add_skim('BtoXgamma', B2XgammaList())


from skim.semileptonic import SemileptonicList
add_skim('SLUntagged', SemileptonicList(path=analysis_main))

from skim.leptonic import LeptonicList
add_skim('LeptonicUntagged', LeptonicList(path=analysis_main))


from skim.semileptonic import PRList
add_skim('PRsemileptonicUntagged', PRList(path=analysis_main))


from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')
# Weightfiles for FEIv4_2018_MC9_release_02_00_01 in this database

import fei
particles = fei.get_default_channels(chargedB=True, neutralB=True, hadronic=True, semileptonic=True, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
analysis_main.add_path(feistate.path)


from skim.fei import *
add_skim('feiHadronicB0', B0hadronic(path=analysis_main))

from skim.fei import*
add_skim('feiHadronicBplus', BplusHadronic(path=analysis_main))

from skim.fei import*
add_skim('feiSLBplusWithOneLep', BplusSLWithOneLep(path=analysis_main))


from skim.fei import*
add_skim('feiSLB0WithOneLep', B0SLWithOneLep(path=analysis_main))


from skim.dark import SinglePhotonDarkList
add_skim('SinglePhotonDark', SinglePhotonDarkList(path=analysis_main))


from skim.dark import LFVZpInvisibleList
add_skim('LFVZpInvisible', LFVZpInvisibleList(path=analysis_main))


from skim.dark import LFVZpVisibleList
add_skim('LFVZpVisible', LFVZpVisibleList(path=analysis_main))

from skim.dark import ALP3GammaList
add_skim('ALP3Gamma', ALP3GammaList(path=analysis_main))

setSkimLogging()
process(analysis_main)

print(statistics)
