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
    ('14140100', 'BtoD0h_hh'),
    ('14140101', 'BtoD0h_Kpi'),
    ('14140102', 'BtoD0h_Kpipipi_Kpipi0'),  # B+ -> anti-D0/anti-D0* (K- pi+ pi+ pi-, K- pi+ pi0) h+
    ('14140200', 'BtoD0h_Kshh'),
    ('14120300', 'BtoD0h_Kspi0'),
    ('14120400', 'BtoD0h_Kspipipi0'),
    # ('14140500', 'BtoD0h_Kspi0pi0'), Add when skim script is ready
    ('14120600', 'B0toDpi_Kpipi'),  # B0 -> D-(k+ pi- pi-)pi+
    ('14120601', 'B0toDpi_Kspi'),   # B0 -> D-(Ks pi-)pi+
    ('14120700', 'B0toDstarPi_D0pi_Kpi'),  # B0 -> D*-(anti-D0 pi-)pi+    With anti-D0 -> k+ pi-
    # merge  B0 -> D*-(anti-D0 pi-)pi+ with anti-D0 -> k- pi+ pi+ pi- and anti-D0 -> K- pi+ pi0
    ('14120800', 'B0toDstarPi_D0pi_Kpipipi_Kpipi0'),
    ('14141000', 'BtoD0rho_Kpi'),
    ('14141001', 'BtoD0rho_Kpipipi_Kpipi0'),
    ('14121100', 'B0toDrho_Kpipi'),
    ('14121101', 'B0toDrho_Kspi'),
    ('14121200', 'B0toDstarRho_D0pi_Kpi'),
    ('14121201', 'B0toDstarRho_D0pi_Kpipipi_Kpipi0'),
    ('11180100', 'feiHadronicB0'),
    ('11180200', 'feiHadronicBplus'),
    ('11180300', 'feiSLB0'),
    ('11180400', 'feiSLBplus'),
    ('12160100', 'BtoXgamma'),
    ('12160200', 'BtoXll'),
    ('12160300', 'BtoXll_LFV'),
    ('14120500', 'BtoPi0Pi0'),
    ('17240100', 'DstToD0Pi_D0ToHpJm'),  # D* -> D0 -> K pi/pi pi/K K
    ('17240200', 'DstToD0Pi_D0ToHpJmPi0'),  # D* -> D0 -> K- pi+ pi0 (RS+WS)
    ('17240300', 'DstToD0Pi_D0ToHpHmPi0'),  # D* -> D0 -> h h pi0
    ('17240400', 'DstToD0Pi_D0ToKsOmega'),  # D* -> D0 -> Ks omega / Ks eta -> Ks pi+ pi- pi0
    ('17240500', 'DstToD0Pi_D0ToHpJmEta'),  # D* -> D0 -> K- pi+ eta (RS+WS)
    ('17240600', 'DstToD0Pi_D0ToNeutrals'),  # D* -> D0 -> pi0 pi0/Ks pi0/Ks Ks
    ('17240700', 'DstToD0Pi_D0ToHpHmKs'),  # D* -> D0 -> h h Ks
    ('17240800', 'EarlyData_DstToD0Pi_D0ToHpJmPi0'),  # D* -> D0 -> K- pi+ pi0 (RS+WS)
    ('17240900', 'EarlyData_DstToD0Pi_D0ToHpHmPi0'),  # D* -> D0 -> h h pi0
    ('17230100', 'XToD0_D0ToHpJm'),  # D0 -> K pi/pi pi/K K
    # ('17230100', 'D0ToHpJm'),  # D0 -> K pi/pi pi/K K
    ('17230200', 'XToD0_D0ToNeutrals'),  # D0 -> pi0 pi0/Ks pi0/Ks Ks
    ('17230300', 'DstToD0Pi_D0ToRare'),  # D0 -> g g/e e/mu mu
    ('17260900', 'DstToD0Pi_D0ToSemileptonic'),  # D*(tag) -> D0(tag) -> hadron, D*(q) -> D0(q) -> K+ nu l-
    ('17230400', 'XToDp_DpToKsHp'),  # D+ -> Ks h+
    ('19130100', 'CharmlessHad2Body'),
    ('19130200', 'CharmlessHad3Body'),
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
    # ('18520300', 'DielectronPlusMissingEnergy'),
    ('18520400', 'LFVZpVisible'),
    ('18530100', 'TwoTrackLeptonsForLuminosity'),
    ('18520500', 'TwoTrackPions'),
    ('18570600', 'TauGeneric'),
    ('18570700', 'TauThrust'),
]

#: A dict listing which standalone skims are included the combined skim scripts
combined_skims = {
    'BtoCharm1': [
        'BtoD0h_Kshh', 'BtoD0h_hh', 'BtoD0h_Kspi0', 'BtoD0h_Kspipipi0'
    ],

    'BtoCharm2': [
        'BtoD0h_Kpi', 'BtoD0h_Kpipipi_Kpipi0', 'B0toDpi_Kpipi', 'B0toDstarPi_D0pi_Kpi', 'B0toDstarPi_D0pi_Kpipipi_Kpipi0'
    ],
    'CharmHigh': [
        'DstToD0Pi_D0ToHpJmPi0', 'XToD0_D0ToNeutrals', 'DstToD0Pi_D0ToHpHmPi0'
    ],
    'CharmLow': [
        'XToD0_D0ToHpJm', 'DstToD0Pi_D0ToHpJm', 'DstToD0Pi_D0ToHpHmKs', 'DstToD0Pi_D0ToRare'
    ],
    'CharmEarlyData': [
        'EarlyData_DstToD0Pi_D0ToHpJmPi0', 'EarlyData_DstToD0Pi_D0ToHpHmPi0'
    ],
    'Dark': [
        'ALP3Gamma', 'SinglePhotonDark', 'LFVZpVisible', 'DimuonPlusMissingEnergy',
        'ElectronMuonPlusMissingEnergy',  # 'DielectronPlusMissingEnergy'
        'TwoTrackLeptonsForLuminosity', 'TwoTrackPions'
    ],
    'EWP': [
        'BtoXll', 'BtoXll_LFV', 'BtoXgamma'
    ],
    'feiHadronicCombined': [
        'feiHadronicB0', 'feiHadronicBplus'
    ],
    'feiSLCombined': [
        'feiSLB0', 'feiSLBplus'
    ],
    'fei': [
        'feiHadronicB0', 'feiHadronicBplus', 'feiSLB0', 'feiSLBplus'
    ],
    'TCPV': [
        'TCPV',
    ],
    'Leptonic': [
        'LeptonicUntagged'
    ],
    'TauCombined': [
        'TauLFV', 'TauGeneric', 'TauThrust'
    ]
}
