import subprocess
from skimExpertFunctions import get_test_file

all_skims = [
    "Dark", "Charm", "BtoCharmless", "BtoCharm", "CombinedSystematics", "EWP", "MiscCombined", "feiHadronic",
    "Semileptonic", "Quarkonium", "feiSLWithOneLepCombined"
]

MCTypes = ['mixedBGx1', 'chargedBGx1', 'ccbarBGx1', 'ssbarBGx1',
           'uubarBGx1', 'ddbarBGx1', 'taupairBGx1',
           'mixedBGx0', 'chargedBGx0', 'ccbarBGx0', 'ssbarBGx0',
           'uubarBGx0', 'ddbarBGx0', 'taupairBGx0']

MCCampaign = 'MC12'

for skim in all_skims:
    for MCType in MCTypes:
        input_file = get_test_file(MCType, MCCampaign)
        script = f"../combined/{skim}_Skim_Standalone.py"
        log_file = f"{skim}_{MCCampaign}_{MCType}.out"
        err_file = f"{skim}_{MCCampaign}_{MCType}.err"
        output_file = f"combinedFiles/{skim}_{MCCampaign}_{MCType}.udst.root"

        print(f'Running {script} on {input_file} (MC type {MCCampaign}_{MCType}) to {output_file}')
        subprocess.run(["bsub", "-q", "l", "-oo", log_file, "-e", err_file, "basf2", script,
                        "-o", output_file, "-i", input_file])
