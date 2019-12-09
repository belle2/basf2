#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2
import modularAnalysis as ma
import b2test_utils
import skimExpertFunctions as expert

from stdCharged import stdPi, stdK, stdE, stdMu, stdPr
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts, mergedKshorts
from skim.standardlists.charm import loadStdD0, loadStdDstar0, loadStdDplus, loadStdDstarPlus
from skim.standardlists.lightmesons import loadStdLightMesons
from skim.standardlists.dileptons import loadStdDiLeptons
b2.set_log_level(b2.LogLevel.INFO)
b2.conditions.disable_globaltag_replay()

skimpath = b2.Path()
ma.inputMdstList('MC9', Belle2.FileSystem.findFile('analysis/tests/mdst.root'), path=skimpath)


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
ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=skimpath)

ma.cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1', path=skimpath)
ma.reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580', path=skimpath)

# ISR cc skim
from skim import quarkonium
expert.add_skim('ISRpipicc', quarkonium.ISRpipiccList(path=skimpath), path=skimpath)

# Tau Skim
from skim import taupair
expert.add_skim('TauLFV', taupair.TauLFVList(1, path=skimpath), path=skimpath)

expert.add_skim('TauGeneric', taupair.TauList(path=skimpath), path=skimpath)


# TCPV Skim
from skim import tcpv
expert.add_skim('TCPV', tcpv.TCPVList(path=skimpath), path=skimpath)

# Bottomonium Etab Skim: 15420100
from skim import quarkonium
expert.add_skim('BottomoniumEtabExclusive', quarkonium.EtabList(path=skimpath), path=skimpath)

expert.add_skim('BottomoniumUpsilon', quarkonium.UpsilonList(path=skimpath), path=skimpath)

# B to Charmless skims
from skim import btocharmless
expert.add_skim('BtoPi0Pi0', btocharmless.BtoPi0Pi0List(path=skimpath), path=skimpath)

Had2BodyList = btocharmless.CharmlessHad2BodyB0List(path=skimpath) + btocharmless.CharmlessHad2BodyBmList(path=skimpath)
expert.add_skim('CharmlessHad2Body', Had2BodyList, path=skimpath)

Had3BodyList = btocharmless.CharmlessHad3BodyB0List(path=skimpath) + btocharmless.CharmlessHad3BodyBmList(path=skimpath)
expert.add_skim('CharmlessHad3Body', Had3BodyList, path=skimpath)


# Charm skims
from skim import charm
# charm Skim: D0 -> K pi/ pi pi /K K
expert.add_skim('XToD0_D0ToHpJm', charm.D0ToHpJm(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> K pi/ pi pi /K K) pi
expert.add_skim('DstToD0Pi_D0ToHpJm', charm.DstToD0PiD0ToHpJm(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> K pi pi0) pi
expert.add_skim('DstToD0Pi_D0ToHpJmPi0', charm.DstToD0PiD0ToHpJmPi0(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> pi pi pi0/K K pi0) pi
expert.add_skim('DstToD0Pi_D0ToHpHmPi0', charm.DstToD0PiD0ToHpHmPi0(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> K pi eta) pi
expert.add_skim('DstToD0Pi_D0ToHpJmEta', charm.DstToD0PiD0ToHpJmEta(path=skimpath), path=skimpath)

# charm Skim: D*+ -> D0 (-> Ks omega/eta (-> pi pi pi0)) pi
expert.add_skim('DstToD0Pi_D0ToKsOmega', charm.DstToD0PiD0ToKsOmega(path=skimpath), path=skimpath)

# charm Skim: D0 -> pi0 pi0/Ks pi0/Ks Ks
expert.add_skim('XToD0_D0ToNeutrals', charm.D0ToNeutrals(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> pi0 pi0/Ks pi0/Ks Ks) pi
expert.add_skim('DstToD0Pi_D0ToNeutrals', charm.DstToD0Neutrals(path=skimpath), path=skimpath)

# charm Skim: D* -> D0 (-> pi pi Ks/ K K Ks) pi
expert.add_skim('DstToD0Pi_D0ToHpHmKs', charm.DstToD0PiD0ToHpHmKs(path=skimpath), path=skimpath)

# charm Skim: D -> gamma gamma/ e e/ mu mu
expert.add_skim('DstToD0Pi_D0ToRare', charm.CharmRare(path=skimpath), path=skimpath)

# charm Skim: D+ -> Ks pi+/ Ks K+
expert.add_skim('XToDp_DpToKsHp', charm.DpToKsHp(path=skimpath), path=skimpath)


# Systematics skim
from skim import systematics
expert.add_skim('Systematics', systematics.SystematicsList(path=skimpath), path=skimpath)

# Systematics Lambda Skim
expert.add_skim('SystematicsLambda', systematics.SystematicsLambdaList(path=skimpath), path=skimpath)

# Systematics Tracking
expert.add_skim('SystematicsTracking', systematics.SystematicsTrackingList(path=skimpath), path=skimpath)

# Resonan ce
expert.add_skim('Resonance', systematics.ResonanceList(path=skimpath), path=skimpath)

# Systematics Rad mu mu
expert.add_skim('SystematicsRadMuMu', systematics.SystematicsRadMuMuList(path=skimpath), path=skimpath)

# Systematics Rad mu mu
expert.add_skim('SystematicsRadEE', systematics.SystematicsRadEEList(path=skimpath), path=skimpath)


# Charm skims
from skim import btocharm
btocharm.loadD0bar(path=skimpath)
expert.add_skim('BtoDh_hh', btocharm.BsigToDhTohhList(path=skimpath), path=skimpath)

# B- to D(->Kshh)h- Skim
btocharm.loadDkshh(path=skimpath)
BtoDhKshhList = btocharm.BsigToDhToKshhList(path=skimpath)
expert.add_skim('BtoDh_Kshh', BtoDhKshhList, path=skimpath)

# B- to D(->Kspi0)h- Skim
btocharm.loadDkspi0(path=skimpath)
BtoDhKspi0List = btocharm.BsigToDhToKspi0List(path=skimpath)
expert.add_skim('BtoDh_Kspi0', BtoDhKspi0List, path=skimpath)

# B- to D(->Kspipipi0)h- Skim
btocharm.loadDkspipipi0(path=skimpath)
BtoDhKspipipi0List = btocharm.BsigToDhToKspipipi0List(path=skimpath)
expert.add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List, path=skimpath)


# EWP Skims
from skim import ewp
expert.add_skim('BtoXll', ewp.B2XllList(path=skimpath), path=skimpath)

expert.add_skim('BtoXgamma', ewp.B2XgammaList(path=skimpath), path=skimpath)


# semileptonic skims
from skim import semileptonic
expert.add_skim('SLUntagged', semileptonic.SemileptonicList(path=skimpath), path=skimpath)

from skim import leptonic
expert.add_skim('LeptonicUntagged', leptonic.LeptonicList(path=skimpath), path=skimpath)

expert.add_skim('PRsemileptonicUntagged', semileptonic.PRList(path=skimpath), path=skimpath)

b2.use_central_database('GT_gen_ana_004.40_AAT-parameters', b2.LogLevel.DEBUG, 'fei_database')
# Weightfiles for FEIv4_2018_MC9_release_02_00_01 in this database

import fei
particles = fei.get_default_channels(chargedB=True, neutralB=True, hadronic=True, semileptonic=True, KLong=False)
configuration = fei.config.FeiConfiguration(prefix='FEIv4_2018_MC9_release_02_00_01', training=False, monitor=False)
feistate = fei.get_path(particles, configuration)
path = skimpath.add_path(feistate.path)


from skim import fei
expert.add_skim('feiHadronicB0', fei.B0Hadronic(path=skimpath), path=skimpath)
expert.add_skim('feiHadronicBplus', fei.BplusHadronic(path=skimpath), path=skimpath)
expert.add_skim('feiSLBplus', fei.BplusSL(path=skimpath), path=skimpath)
expert.add_skim('feiSLB0', fei.B0SL(path=skimpath), path=skimpath)

# Dark Sector Skims
from skim import dark
expert.add_skim('SinglePhotonDark', dark.SinglePhotonDarkList(path=skimpath), path=skimpath)
expert.add_skim('ALP3Gamma', dark.ALP3GammaList(path=skimpath), path=skimpath)
expert.add_skim('DimuonPlusMissingEnergy', dark.DimuonPlusMissingEnergyList(path=skimpath), path=skimpath)
expert.add_skim('ElectronMuonPlusMissingEnergy', dark.ElectronMuonPlusMissingEnergyList(path=skimpath), path=skimpath)
expert.add_skim('DielectronPlusMissingEnergy', dark.DielectronPlusMissingEnergyList(path=skimpath), path=skimpath)
expert.add_skim('LFVZpVisible', dark.LFVZpVisibleList(path=skimpath), path=skimpath)

expert.setSkimLogging(path=skimpath)

# process the basf2 path in a temporary directory (so all of the skimmed udst
# files get cleaned up afterwards).
with b2test_utils.clean_working_directory():
    b2.process(skimpath, 1)  # just process one event

print(b2.statistics)
