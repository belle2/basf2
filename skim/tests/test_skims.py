#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import b2test_utils
import basf2
from ROOT import Belle2
from modularAnalysis import *

from stdCharged import stdPi, stdK, stdE, stdMu, stdPr
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
set_log_level(LogLevel.INFO)

from skimExpertFunctions import setSkimLogging, add_skim
skimpath = Path()
inputMdstList('MC9', Belle2.FileSystem.findFile('analysis/tests/mdst.root'), path=skimpath)


stdPi0s('loose', path=skimpath)
stdPhotons('loose', path=skimpath)
stdKshorts(path=skimpath)
mergedKshorts(path=skimpath)  # add due to charm skims need it
stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdPr('loose', path=skimpath)
stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
stdE('all', path=skimpath)
stdMu('all', path=skimpath)

loadStdLightMesons(path=skimpath)
loadStdSkimPi0(path=skimpath)
loadStdSkimPhoton(path=skimpath)
stdPhotons('all', path=skimpath)
stdPhotons('tight', path=skimpath)  # also builds loose list
stdK('95eff', path=skimpath)
stdPi('95eff', path=skimpath)
stdE('95eff', path=skimpath)
stdMu('95eff', path=skimpath)
stdPr('90eff', path=skimpath)

loadStdD0(path=skimpath)
loadStdDplus(path=skimpath)
loadStdDstar0(path=skimpath)
loadStdDstarPlus(path=skimpath)
loadStdDiLeptons(True, path=skimpath)
cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=skimpath)

cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1', path=skimpath)
reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580', path=skimpath)

# ISR cc skim
from skim.quarkonium import ISRpipiccList
add_skim('ISRpipicc', ISRpipiccList(path=skimpath), path=skimpath)

# BtoPi0Pi0 Skim
from skim.btocharmless import BtoPi0Pi0List
add_skim('BtoPi0Pi0', BtoPi0Pi0List(path=skimpath), path=skimpath)

# Tau Skim
from skim.taupair import TauLFVList
add_skim('TauLFV', TauLFVList(1, path=skimpath), path=skimpath)

from skim.taupair import TauList
add_skim('TauGeneric', TauList(path=skimpath), path=skimpath)


# TCPV Skim
from skim.tcpv import TCPVList
add_skim('TCPV', TCPVList(path=skimpath), path=skimpath)

# Bottomonium Etab Skim: 15420100
from skim.quarkonium import *
add_skim('BottomoniumEtabExclusive', EtabList(path=skimpath), path=skimpath)

from skim.quarkonium import *
add_skim('BottomoniumUpsilon', UpsilonList(path=skimpath), path=skimpath)

# B to Charmless skim
from skim.btocharmless import CharmlessHad2BodyB0List, CharmlessHad2BodyBmList
Had2BodyList = CharmlessHad2BodyB0List(path=skimpath) + CharmlessHad2BodyBmList(path=skimpath)
add_skim('CharmlessHad2Body', Had2BodyList, path=skimpath)

from skim.btocharmless import CharmlessHad3BodyB0List, CharmlessHad3BodyBmList
Had3BodyList = CharmlessHad3BodyB0List(path=skimpath) + CharmlessHad3BodyBmList(path=skimpath)
add_skim('CharmlessHad3Body', Had3BodyList, path=skimpath)


# Charm skims
# charm Skim: D0 -> K pi/ pi pi /K K
from skim.charm import D0ToHpJm
add_skim('Charm2BodyHadronicD0', D0ToHpJm(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> K pi/ pi pi /K K) pi
from skim.charm import DstToD0PiD0ToHpJm
add_skim('Charm2BodyHadronic', DstToD0PiD0ToHpJm(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> K pi pi0) pi
from skim.charm import DstToD0PiD0ToHpJmPi0
add_skim('Charm3BodyHadronic', DstToD0PiD0ToHpJmPi0(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> pi pi pi0/K K pi0) pi
from skim.charm import DstToD0PiD0ToHpHmPi0
add_skim('Charm3BodyHadronic1', DstToD0PiD0ToHpHmPi0(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> K pi eta) pi
from skim.charm import DstToD0PiD0ToHpJmEta
add_skim('Charm3BodyHadronic3', DstToD0PiD0ToHpJmEta(path=skimpath), path=skimpath)

# charm Skim: D*+ -> D0 (-> Ks omega/eta (-> pi pi pi0)) pi
from skim.charm import DstToD0PiD0ToKsOmega
add_skim('Charm2BodyNeutrals2', DstToD0PiD0ToKsOmega(path=skimpath), path=skimpath)

# charm Skim: D0 -> pi0 pi0/Ks pi0/Ks Ks
from skim.charm import D0ToNeutrals
add_skim('Charm2BodyNeutralsD0', D0ToNeutrals(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> pi0 pi0/Ks pi0/Ks Ks) pi
from skim.charm import DstToD0Neutrals
add_skim('Charm2BodyNeutrals', DstToD0Neutrals(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> pi pi Ks/ K K Ks) pi
from skim.charm import DstToD0PiD0ToHpHmKs
add_skim('Charm3BodyHadronic2', DstToD0PiD0ToHpHmKs(path=skimpath), path=skimpath)

# charm Skim: D -> gamma gamma/ e e/ mu mu
from skim.charm import CharmRareList
add_skim('CharmRare', CharmRareList(path=skimpath), path=skimpath)

from skim.charm import CharmSemileptonicList
add_skim('CharmSemileptonic', CharmSemileptonicList(path=skimpath), path=skimpath)


# Systematics skim
from skim.systematics import SystematicsList
add_skim('Systematics', SystematicsList(path=skimpath), path=skimpath)

# Systematics Lambda Skim
from skim.systematics import SystematicsLambdaList
add_skim('SystematicsLambda', SystematicsLambdaList(path=skimpath), path=skimpath)

# Systematics Tracking
from skim.systematics import SystematicsTrackingList
add_skim('SystematicsTracking', SystematicsTrackingList(path=skimpath), path=skimpath)

# Resonan ce
from skim.systematics import ResonanceList
add_skim('Resonance', ResonanceList(path=skimpath), path=skimpath)

# Systematics Rad mu mu
from skim.systematics import SystematicsRadMuMuList
add_skim('SystematicsRadMuMu', SystematicsRadMuMuList(path=skimpath), path=skimpath)

# Systematics Rad mu mu
from skim.systematics import SystematicsRadEEList
add_skim('SystematicsRadEE', SystematicsRadEEList(path=skimpath), path=skimpath)


from skim.btocharm import BsigToDhTohhList, loadD0bar
loadD0bar(path=skimpath)
add_skim('BtoDh_hh', BsigToDhTohhList(path=skimpath), path=skimpath)


# B- to D(->Kshh)h- Skim
from skim.btocharm import BsigToDhToKshhList, loadDkshh
loadDkshh(path=skimpath)
BtoDhKshhList = BsigToDhToKshhList(path=skimpath)
add_skim('BtoDh_Kshh', BtoDhKshhList, path=skimpath)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import BsigToDhToKspi0List, loadDkspi0
loadDkspi0(path=skimpath)
BtoDhKspi0List = BsigToDhToKspi0List(path=skimpath)
add_skim('BtoDh_Kspi0', BtoDhKspi0List, path=skimpath)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import BsigToDhToKspipipi0List, loadDkspipipi0
loadDkspipipi0(path=skimpath)
BtoDhKspipipi0List = BsigToDhToKspipipi0List(path=skimpath)
add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List, path=skimpath)


# EWP Skims
from skim.ewp import B2XllList
add_skim('BtoXll', B2XllList(path=skimpath), path=skimpath)


from skim.ewp import B2XgammaList
add_skim('BtoXgamma', B2XgammaList(path=skimpath), path=skimpath)


# semileptonic skims
from skim.semileptonic import SemileptonicList
add_skim('SLUntagged', SemileptonicList(path=skimpath), path=skimpath)

from skim.leptonic import LeptonicList
add_skim('LeptonicUntagged', LeptonicList(path=skimpath), path=skimpath)


from skim.semileptonic import PRList
add_skim('PRsemileptonicUntagged', PRList(path=skimpath), path=skimpath)


from fei import backward_compatibility_layer
backward_compatibility_layer.pid_renaming_oktober_2017()
use_central_database('GT_gen_ana_004.40_AAT-parameters', LogLevel.DEBUG, 'fei_database')
# Weightfiles for FEIv4_2018_MC9_release_02_00_01 in this database

import fei
particles = fei.get_default_channels(chargedB=True, neutralB=True, hadronic=True, semileptonic=True, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
path = skimpath.add_path(feistate.path)


from skim.fei import *
add_skim('feiHadronicB0', B0hadronic(path=skimpath), path=skimpath)

from skim.fei import*
add_skim('feiHadronicBplus', BplusHadronic(path=skimpath), path=skimpath)

from skim.fei import*
add_skim('feiSLBplusWithOneLep', BplusSLWithOneLep(path=skimpath), path=skimpath)


from skim.fei import*
add_skim('feiSLB0WithOneLep', B0SLWithOneLep(path=skimpath), path=skimpath)


from skim.dark import SinglePhotonDarkList
add_skim('SinglePhotonDark', SinglePhotonDarkList(path=skimpath), path=skimpath)


from skim.dark import LFVZpInvisibleList
add_skim('LFVZpInvisible', LFVZpInvisibleList(path=skimpath), path=skimpath)


from skim.dark import LFVZpVisibleList
add_skim('LFVZpVisible', LFVZpVisibleList(path=skimpath), path=skimpath)

from skim.dark import ALP3GammaList
add_skim('ALP3Gamma', ALP3GammaList(path=skimpath), path=skimpath)

setSkimLogging(path=skimpath)

# process the basf2 path in a temporary directory (so all of the skimmed udst
# files get cleaned up afterwards).
with b2test_utils.clean_working_directory():
    basf2.process(skimpath, 1)  # just process one event

print(statistics)
