import subprocess
from skimExpertFunctions import get_test_file

all_skims = [
    'ALP3Gamma', 'BottomoniumEtabExclusive', 'BottomoniumUpsilon',
    'BtoDh_Kspipipi0', 'BtoDh_Kspi0', 'BtoDh_hh', 'BtoDh_Kshh',
    'BtoPi0Pi0', 'BtoXll', 'BtoXgamma', 'Charm2BodyHadronic', 'Charm2BodyHadronicD0',
    'Charm2BodyNeutrals', 'Charm2BodyNeutralsD0', 'Charm3BodyHadronic', 'Charm3BodyHadronic2',
    'Charm3BodyHadronic1', 'Charm3BodyHadronic3', 'CharmRare', 'CharmSemileptonic',
    'CharmlessHad2Body', 'CharmlessHad3Body', 'ISRpipicc',
    'LFVZpInvisible', 'LFVZpVisible', 'LeptonicUntagged', 'PRsemileptonicUntagged',
    'SLUntagged', 'SinglePhotonDark', 'SystematicsEELL', 'SystematicsRadMuMu', 'SystematicsRadEE',
    'SystematicsLambda', 'Systematics', 'SystematicsTracking', 'TauLFV', 'TCPV', 'TauGeneric',
    'feiHadronicB0', 'feiHadronicBplus', 'feiSLB0WithOneLep', 'feiSLBplusWithOneLep'
]

MCTypes = ['mixedBGx1', 'chargedBGx1', 'ccbarBGx1', 'ssbarBGx1',
           'uubarBGx1', 'ddbarBGx1', 'taupairBGx1',
           'mixedBGx0', 'chargedBGx0', 'ccbarBGx0', 'ssbarBGx0',
           'uubarBGx0', 'ddbarBGx0', 'taupairBGx0']

MCCampaign = 'MC12'

for skim in skims:
    for MCType in MCTypes:
        input_file = get_test_file(MCType, MCCampaign)
        script = f'../standalone/{skim}_Skim_Standalone.py'
        log_file = f'{skim}_{MCType}.out'
        err_file = f'{skim}_{MCType}.err'
        output_file = f'{skim}_{MCType}.udst.root'

        print(f'Running {script} on {input_file} (MC type {MCCampaign}_{MCType}) to {output_file}')
        subprocess.run(['bsub', '-q', 'l', '-oo', log_file, '-e', err_file, 'basf2', script,
                        '-o', output_file, '-i', input_file])
