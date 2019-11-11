from pathlib import Path
import subprocess
from tempfile import TemporaryNamedFile

from basf2 import find_file
from skimExpertFunctions import get_test_file

statsDirectory = find_file('skim/tools/stats')

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


MCTypes = ['mixedBGx1', 'chargedBGx1', 'ccbarBGx1', 'ssbarBGx1',
           'uubarBGx1', 'ddbarBGx1', 'taupairBGx1',
           'mixedBGx0', 'chargedBGx0', 'ccbarBGx0', 'ssbarBGx0',
           'uubarBGx0', 'ddbarBGx0', 'taupairBGx0']

MCCampaign = 'MC12'

for skim in all_skims:
    for MCType in MCTypes:
        input_file = get_test_file(f'{MCType}_{MCCampaign}')
        script = find_file(f'skim/standalone/{skim}_Skim_Standalone.py')

        Path(statsDirectory, 'log').mkdir(parents=True, exist_ok=True)
        log_file = Path(statsDirectory, 'log', f'{skim}_{MCCampaign}_{MCType}.out')
        err_file = Path(statsDirectory, 'log', f'{skim}_{MCCampaign}_{MCType}.err')
        json_file = Path(statsDirectory, 'log', f'JobInformation_{skim}_{MCCampaign}_{MCType}.json')
        output_file = TemporaryNamedFile().name

        print(f'Running {script} on {input_file} (MC type {MCCampaign}_{MCType}) to {output_file}')
        subprocess.run(['bsub', '-q', 'l', '-oo', log_file, '-e', err_file, 'basf2', script,
                        '--job-information', json_file,
                        '-n', '10000',
                        '-o', output_file, '-i', input_file])
