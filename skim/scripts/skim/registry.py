# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Registry
~~~~~~~~

All skims must be registered and encoded by the relevant skim liaison.
Registering a skim is as simple as adding it to the list in ``registry.py``.

For information about the convention: https://confluence.desy.de/x/URdYBQ

"""

#: A list of all official registered skims and their codes
skim_registry = [
    ('11110100', 'PRsemileptonicUntagged'),
    ('15440100', 'BottomoniumUpsilon'),
    ('15420100', 'BottomoniumEtabExclusive'),
    ('11160200', 'SLUntagged'),
    ('11130300', 'LeptonicUntagged'),
    ('14140100', 'BtoDh_hh'),
    ('14120300', 'BtoDh_Kspi0'),
    ('14140200', 'BtoDh_Kshh'),
    ('14120400', 'BtoDh_Kspipipi0'),
    ('11180100', 'feiHadronicB0'),
    ('11180200', 'feiHadronicBplus'),
    ('11180300', 'feiSLB0'),
    ('11180400', 'feiSLBplus'),
    ('12160100', 'BtoXgamma'),
    ('12160200', 'BtoXll'),
    ('12160300', 'BtoXll_LFV'),
    ('14120500', 'BtoPi0Pi0'),
    ('17240100', 'DstToD0Pi_D0ToHpJm'),  # D* -> D0 -> K pi/pi pi/K K
    # ('17240100', 'DstToD0PiD0ToHpJm'),  # D* -> D0 -> K pi/pi pi/K K
    ('17240200', 'DstToD0Pi_D0ToHpJmPi0'),  # D* -> D0 -> K- pi+ pi0 (RS+WS)
    ('17240300', 'DstToD0Pi_D0ToHpHmPi0'),  # D* -> D0 -> h h pi0
    ('17240400', 'DstToD0Pi_D0ToKsOmega'),  # D* -> D0 -> Ks omega / Ks eta -> Ks pi+ pi- pi0
    ('17240500', 'DstToD0Pi_D0ToHpJmEta'),  # D* -> D0 -> K- pi+ eta (RS+WS)
    ('17240600', 'DstToD0Pi_D0ToNeutrals'),  # D* -> D0 -> pi0 pi0/Ks pi0/Ks Ks
    ('17240700', 'DstToD0Pi_D0ToHpHmKs'),  # D* -> D0 -> h h Ks
    ('17230100', 'XToD0_D0ToHpJm'),  # D0 -> K pi/pi pi/K K
    # ('17230100', 'D0ToHpJm'),  # D0 -> K pi/pi pi/K K
    ('17230200', 'XToD0_D0ToNeutrals'),  # D0 -> pi0 pi0/Ks pi0/Ks Ks
    ('17230300', 'DstToD0Pi_D0ToRare'),  # D0 -> g g/e e/mu mu
    ('17260900', 'DstToD0Pi_D0ToSemileptonic'),  # D*(tag) -> D0(tag) -> hadron, D*(q) -> D0(q) -> K+ nu l-
    ('17230400', 'XToDp_DpToKsHp'),  # D+ -> Ks h+
    ('19130100', 'CharmlessHad2Body'),
    ('19130200', 'CharmlessHad3Body'),
    ('14130200', 'DoubleCharm'),
    ('16460100', 'ISRpipicc'),
    ('10600100', 'Systematics'),
    ('10620200', 'SystematicsLambda'),
    ('10600300', 'SystematicsTracking'),
    ('10600400', 'Resonance'),
    ('10600500', 'SystematicsRadMuMu'),
    ('10600600', 'SystematicsEELL'),
    ('10600700', 'SystematicsRadEE'),
    ('18360100', 'TauLFV'),
    ('13160100', 'TCPV'),
    ('18020100', 'SinglePhotonDark'),
    ('18020300', 'ALP3Gamma'),
    ('18520100', 'DimuonPlusMissingEnergy'),
    ('18520200', 'ElectronMuonPlusMissingEnergy'),
    ('18520300', 'DielectronPlusMissingEnergy'),
    ('18520400', 'LFVZpVisible'),
    ('18570600', 'TauGeneric'),
    ('18570700', 'TauThrust'),
]
