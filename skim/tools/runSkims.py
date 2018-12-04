import subprocess
from skimExpertFunctions import *

all_skims = [
    "Dark", "BtoCharmless", "BtoCharm", "ALP3Gamma",
    "BottomoniumEtabExclusive", "BottomoniumUpsilon", "TauGeneric",
    "SystematicsRadMuMu", "SystematicsRadEE", "LFVZpInvisible", "LFVZpVisible",
    "SinglePhotonDark", "SystematicsTracking", "BottomoniumUpsilon",
    "BottomoniumEtabExclusive", "SystematicsLambda", "Systematics",
    "Resonance", "ISRpipicc", "BtoDh_Kspipipi0", "BtoPi0Pi0",
    "CharmSemileptonic", "BottomoniumEtabExclusive", "BottomoniumUpsilon",
    "feiSLB0WithOneLep", "feiBplusWithOneLep", "feiHadronicB0",
    "feiHadronicBplus", "BtoPi0Pi0", "Charm3BodyHadronic2",
    "Charm3BodyHadronic", "Charm3BodyHadronicD0", "Charm2BodyHadronic",
    "Charm2BodyNeutrals", "Charm2BodyNeutralsD0", "BtoDh_Kspi0", "BtoDh_hh",
    "BtoDh_Kshh", "Tau", "PRsemileptonicUntagged", "SLUntagged",
    "LeptonicUntagged", "TCPV", "CharmRare", "BtoXll", "BtoXgamma", "TauLFV",
]
all_data_types = {

    # MC10 generic samples
    "MC10_mixedBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004770/s00/e0000/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000001_prod00004770_task00000001.root',
    "MC10_mixedBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003591/s00/e0000/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000001_prod00003591_task00000001.root',
    "MC10_chargedBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004771/s00/e0000/4S/r00000/charged/' +
    'mdst/sub00/mdst_000001_prod00004771_task00000001.root',
    "MC10_chargedBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003593/s00/e0000/4S/r00000/charged/' +
    'mdst/sub00/mdst_000001_prod00003593_task00000001.root',
    "MC10_uubarBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004772/s00/e0000/4S/r00000/uubar/' +
    'mdst/sub00/mdst_000001_prod00004772_task00000001.root',
    "MC10_uubarBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003595/s00/e0000/4S/r00000/uubar/' +
    'mdst/sub00/mdst_000001_prod00003595_task00000001.root',
    "MC10_ccbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004775/s00/e0000/4S/r00000/ccbar/' +
    'mdst/sub00/mdst_000001_prod00004775_task00000001.root',
    "MC10_ccbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003601/s00/e0000/4S/r00000/ccbar/' +
    'mdst/sub00/mdst_000001_prod00003601_task00000001.root',
    "MC10_ddbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004773/s00/e0000/4S/r00000/ddbar/' +
    'mdst/sub00/mdst_000001_prod00004773_task00000001.root',
    "MC10_ddbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003597/s00/e0000/4S/r00000/ddbar/' +
    'mdst/sub00/mdst_000001_prod00003597_task00000001.root',
    "MC10_ssbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004774/s00/e0000/4S/r00000/ssbar/' +
    'mdst/sub00/mdst_000001_prod00004774_task00000001.root',
    "MC10_ssbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003599/s00/e0000/4S/r00000/ssbar/' +
    'mdst/sub00/mdst_000001_prod00003599_task00000001.root',
    "MC10_taupairBGx1": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00004776/s00/e0000/4S/r00000/taupair/' +
    'mdst/sub00/mdst_000001_prod00004776_task00000001.root',
    "MC10_taupairBGx0": '/ghi/fs01/belle2/bdata/MC/release-01-00-03/DB00000294/MC10/prod00003603/s00/e0000/4S/r00000/taupair/' +
    'mdst/sub00/mdst_000001_prod00003603_task00000001.root',

    # MC9 generic samples
    "MC9_mixedBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root',
    "MC9_chargedBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002289/e0000/4S/r00000/charged/sub00/' +
    'mdst_000001_prod00002289_task00000001.root',
    "MC9_ccbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002321/e0000/4S/r00000/ccbar/sub00/' +
    'mdst_000001_prod00002321_task00000001.root',
    "MC9_ssbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002312/e0000/4S/r00000/ssbar/sub00/' +
    'mdst_000001_prod00002312_task00000001.root',
    "MC9_uubarBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002290/e0000/4S/r00000/uubar/sub00/' +
    'mdst_000001_prod00002290_task00000001.root',
    "MC9_ddbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002311/e0000/4S/r00000/ddbar/sub00/' +
    'mdst_000268_prod00002311_task00000268.root',
    "MC9_taupairBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002322/e0000/4S/r00000/taupair/sub00/' +
    'mdst_000001_prod00002322_task00000001.root',
    "MC9_mixedBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002166/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002166_task00000001.root',
    "MC9_chargedBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002167/e0000/4S/r00000/charged/sub00/' +
    'mdst_000001_prod00002167_task00000001.root',
    "MC9_ccbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002171/e0000/4S/r00000/ccbar/sub00/' +
    'mdst_000001_prod00002171_task00000001.root',
    "MC9_ssbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002170/e0000/4S/r00000/ssbar/sub00/' +
    'mdst_000001_prod00002170_task00000001.root',
    "MC9_uubarBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002168/e0000/4S/r00000/uubar/sub00/' +
    'mdst_000001_prod00002168_task00000001.root',
    "MC9_ddbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002169/e0000/4S/r00000/ddbar/sub00/' +
    'mdst_000001_prod00002169_task00000001.root',
    "MC9_taupairBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-00/DB00000265/MC9/prod00002172/e0000/4S/r00000/taupair/sub00/' +
    'mdst_000001_prod00002172_task00000001.root',
    # MC9 3S samples
    "ccbar3SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002709/e0000/3S/r00000/ccbar/sub00/' +
    'mdst_000001_prod00002709_task00000001.root',
    "generic3SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002705/e0000/3S/r00000/generic/sub00/' +
    'mdst_000001_prod00002705_task00000001.root',
    "ddbar3SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002707/e0000/3S/r00000/ddbar/sub00/' +
    'mdst_000001_prod00002707_task00000001.root',
    "MC9_ssbar3SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002708/e0000/3S/r00000/ssbar/sub00/' +
    'mdst_000001_prod00002708_task00000001.root',
    "MC9_taupair3SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002714/e0000/3S/r00000/taupair/sub00/' +
    'mdst_000001_prod00002714_task00000001.root',
    "MC9_uubar3SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002706/e0000/3S/r00000/uubar/sub00/' +
    'mdst_000001_prod00002706_task00000001.root',
    # MC9 5S samples
    "MC9_bsbs5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002281/e0000/5S/r00000/bsbs/sub00/' +
    'mdst_000002_prod00002281_task00000002.root',
    "MC9_nonbsbs5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002282/e0000/5S/r00000/nonbsbs/sub00/' +
    'mdst_000002_prod00002282_task00000002.root',
    "MC9_uubar5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002283/e0000/5S/r00000/uubar/sub00/' +
    'mdst_000001_prod00002283_task00000001.root',
    "ddbar5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002284/e0000/5S/r00000/ddbar/sub00/' +
    'mdst_000001_prod00002284_task00000001.root',
    "MC9_ssbar5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002285/e0000/5S/r00000/ssbar/sub00/' +
    'mdst_000001_prod00002285_task00000001.root',
    "ccbar5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002286/e0000/5S/r00000/ccbar/sub00/' +
    'mdst_000001_prod00002286_task00000001.root',
    "MC9_taupair5SBGx0": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002287/e0000/5S/r00000/taupair/sub00/' +
    'mdst_000001_prod00002287_task00000001.root',
    "MC9_bsbs5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002664/e0000/5S/r00000/bsbs/sub00/' +
    'mdst_000001_prod00002664_task00000001.root',
    "MC9_nonbsbs5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002665/e0000/5S/r00000/nonbsbs/sub00/' +
    'mdst_000001_prod00002665_task00000001.root',
    "MC9_uubar5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002667/e0000/5S/r00000/uubar/sub00/' +
    'mdst_000001_prod00002667_task00000001.root',
    "ddbar5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002669/e0000/5S/r00000/ddbar/sub00/' +
    'mdst_000001_prod00002669_task00000001.root',
    "MC9_ssbar5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002670/e0000/5S/r00000/ssbar/sub00/' +
    'mdst_000001_prod00002670_task00000001.root',
    "ccbar5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002671/e0000/5S/r00000/ccbar/sub00/' +
    'mdst_000001_prod00002671_task00000001.root',
    "MC9_taupair5SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002698/e0000/5S/r00000/taupair/sub00/' +
    'mdst_000001_prod00002698_task00000001.root',
    # 6S samples,
    "MC9_bsbs6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002715/e0000/6S/r00000/bsbs/sub00/' +
    'mdst_000001_prod00002715_task00000001.root',
    "MC9_nonbsbs6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002716/e0000/6S/r00000/nonbsbs/sub00/' +
    'mdst_000001_prod00002716_task00000001.root',
    "ccbar6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002702/e0000/6S/r00000/ccbar/sub00/' +
    'mdst_000001_prod00002702_task00000001.root',
    "MC9_uubar6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002699/e0000/6S/r00000/uubar/sub00/' +
    'mdst_000001_prod00002699_task00000001.root',
    "ddbar6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002700/e0000/6S/r00000/ddbar/sub00/' +
    'mdst_000001_prod00002700_task00000001.root',
    "MC9_ssbar6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002701/e0000/6S/r00000/ssbar/sub00/' +
    'mdst_000001_prod00002701_task00000001.root',
    "MC9_taupair6SBGx1": '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002703/e0000/6S/r00000/taupair/sub00/' +
    'mdst_000001_prod00002703_task00000001.root',
}

for skim in all_skims:
    for dtype, input_file in all_data_types.items():
        print(skim)
        print(dtype)
        script = f"../standalone/{skim}_Skim_Standalone.py"
        log_file = f"{skim}_{dtype}.out"
        err_file = f"{skim}_{dtype}.err"
        output_file = f"{skim}_{dtype}.udst.root"

        print(f"Running {script} on {input_file} to {output_file}")
        subprocess.run(["bsub", "-q", "l", "-oo", log_file, "-e", err_file, "basf2", script,
                        "-o", output_file, "-n", "10000", "-i", input_file])
