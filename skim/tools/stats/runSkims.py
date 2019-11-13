#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
"""

from pathlib import Path
import subprocess
from tempfile import NamedTemporaryFile

from basf2 import find_file
from skimExpertFunctions import get_test_file


all_skims = [
    'ALP3Gamma', 'BottomoniumEtabExclusive', 'BottomoniumUpsilon',
    'BtoDh_Kspipipi0', 'BtoDh_Kspi0', 'BtoDh_hh', 'BtoDh_Kshh',
    'BtoPi0Pi0', 'BtoXll', 'BtoXgamma', 'BtoXll_LFV',
    'DielectronPlusMissingEnergy', 'DimuonPlusMissingEnergy', 'ElectronMuonPlusMissingEnergy'
    'DstToD0Pi_D0ToHpJm', 'XToD0_D0ToHpJm', 'DstToD0Pi_D0ToKsOmega'
    'DstToD0Pi_D0ToNeutrals', 'XToD0_D0ToNeutrals', 'DstToD0Pi_D0ToHpJmPi0', 'DstToD0Pi_D0ToHpHmKs',
    'DstToD0Pi_D0ToHpHmPi0', 'DstToD0Pi_D0ToHpJmEta', 'DstToD0Pi_D0ToRare', 'DstToD0Pi_D0ToSemileptonic',
    'XToDp_DpToKsHp', 'CharmlessHad2Body', 'CharmlessHad3Body', 'ISRpipicc',
    'LFVZpVisible', 'LeptonicUntagged', 'PRsemileptonicUntagged',
    'SLUntagged', 'SinglePhotonDark',
    'SystematicsEELL', 'SystematicsRadMuMu', 'SystematicsRadEE',
    'SystematicsLambda', 'Systematics', 'SystematicsTracking', 'Resonance',
    'TauThrust', 'TauLFV', 'TCPV', 'TauGeneric',
    'feiHadronicB0', 'feiHadronicBplus', 'feiSLB0', 'feiSLBplus'
]


mcCampaign = 'MC12'
beamBackgrounds = ['BGx1', 'BGx0']
mcSampleTypes = ['mixed', 'charged', 'ccbar', 'uubar', 'ddbar', 'ssbar', 'taupair']

mcSamples = [f'{mcCampaign}_{mcSample}{beamBackground}'
             for beamBackground in beamBackgrounds
             for mcSample in mcSampleTypes]

dataSamples = ['proc9_exp3', 'proc9_exp7', 'proc9_exp8', 'bucket7_exp8']

samples = mcSamples + dataSamples

for skim in all_skims:
    for sample in samples:
        input_file = get_test_file(sample)
        script = find_file(f'skim/standalone/{skim}_Skim_Standalone.py')

        Path('log').mkdir(parents=True, exist_ok=True)
        log_file = Path('log', f'{skim}_{sample}.out')
        err_file = Path('log', f'{skim}_{sample}.err')
        json_file = Path('log', f'JobInformation_{skim}_{sample}.json')
        output_file = NamedTemporaryFile().name

        print(f'Running {script} on {input_file} (sample label: {sample}) to {output_file}')
        subprocess.run(['bsub', '-q', 'l', '-oo', log_file, '-e', err_file, 'basf2', script,
                        '--job-information', json_file,
                        '-n', '10000',
                        '-o', output_file, '-i', input_file])
