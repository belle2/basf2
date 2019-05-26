import subprocess
from skimExpertFunctions import *

all_skims = [
    "Dark", "Charm", "BtoCharmless", "BtoCharm", "CombinedSystematics", "EWP", "MiscCombined", "feiHadronic",
    "Semileptonic", "Quarkonium", "feiSLWithOneLepCombined"
]


MC12_types = {


    "MC12_mixedBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000141_prod00007392_task10020000141.root',
    "MC12_chargedBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007394/s00/e1003/4S/r00000/charged/' +
    'mdst/sub00/mdst_000225_prod00007394_task10020000225.root',
    "MC12_ccbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007426/s00/e1003/4S/r00000/ccbar/' +
    'mdst/sub00/mdst_000262_prod00007426_task10020000262.root',
    "MC12_uubarBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007396/s00/e1003/4S/r00000/uubar/' +
    'mdst/sub00/mdst_000621_prod00007396_task10020000627.root',
    "MC12_ddbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007398/s00/e1003/4S/r00000/ddbar/' +
    'mdst/sub00/mdst_000068_prod00007398_task10020000068.root',
    "MC12_ssbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007400/s00/e1003/4S/r00000/ssbar/' +
    'mdst/sub00/mdst_000141_prod00007400_task10020000142.root',
    "MC12_taupairBGx1": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007404/s00/e1003/4S/r00000/taupair/' +
    'mdst/sub00/mdst_000171_prod00007404_task10020000172.root',
    "MC12_mixedBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007393/s00/e1003/4S/r00000/mixed/' +
    'mdst_000018_prod00007393_task10020000018.root',
    "MC12_chargedBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007395/s00/e1003/4S/r00000/charged/' +
    'mdst/sub00/mdst_000007_prod00007395_task10020000007.root',
    "MC12_ccbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007403/s00/e1003/4S/r00000/ccbar/' +
    'mdst/sub00/mdst_000053_prod00007403_task10020000053.root',
    "MC12_uubarBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007397/s00/e1003/4S/r00000/uubar/' +
    'mdst/sub00/mdst_000074_prod00007397_task10020000074.root',
    "MC12_ddbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007399/s00/e1003/4S/r00000/ddbar/' +
    'mdst/sub00/mdst_000006_prod00007399_task10020000006.root',
    "MC12_ssbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007401/s00/e1003/4S/r00000/ssbar/' +
    'mdst/sub00/mdst_000025_prod00007401_task10020000025.root',
    "MC12_taupairBGx0": '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007405/s00/e1003/4S/r00000/taupair/' +
    'mdst/sub00/mdst_000034_prod00007405_task10020000034.root',

}


MC11_types = {

    "MC11_mixedBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005678/s00/e0000/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000995_prod00005678_task00000995.root',
    "MC11_chargedBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005679/s00/e0000/4S/r00000/charged/' +
    'mdst/sub00/mdst_000990_prod00005679_task00000992.root',
    "MC11_ccbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005683/s00/e0000/4S/r00000/ccbar/' +
    'mdst/sub00/mdst_000994_prod00005683_task00001024.root',
    "MC11_uubarBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005680/s00/e0000/4S/r00000/uubar/' +
    'mdst/sub00/mdst_000962_prod00005680_task00000976.root',
    "MC11_ddbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005681/s00/e0000/4S/r00000/ddbar/' +
    'mdst/sub00/mdst_000995_prod00005681_task00000996.root',
    "MC11_ssbarBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005682/s00/e0000/4S/r00000/ssbar/' +
    'mdst/sub00/mdst_000995_prod00005681_task00000996.root',
    "MC11_taupairBGx1": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005684/s00/e0000/4S/r00000/taupair/' +
    'mdst/sub00/mdst_000999_prod00005684_task00001002.root',
    "MC11_mixedBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006102/s00/e0000/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000001_prod00006102_task00000001.root',
    "MC11_chargedBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006103/s00/e0000/4S/r00000/charged/' +
    'mdst/sub00/mdst_000001_prod00006103_task00000001.root',
    "MC11_ccbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006107/s00/e0000/4S/r00000/ccbar/' +
    'mdst/sub00/mdst_000001_prod00006107_task00000001.root',
    "MC11_uubarBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006104/s00/e0000/4S/r00000/uubar/' +
    'mdst/sub00/mdst_000001_prod00006104_task00000001.root',
    "MC11_ddbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006105/s00/e0000/4S/r00000/ddbar/' +
    'mdst/sub00/mdst_000486_prod00006105_task00000491.root',
    "MC11_ssbarBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006106/s00/e0000/4S/r00000/ssbar/' +
    'mdst/sub00/mdst_000001_prod00006106_task00000001.root',
    "MC11_taupairBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00006108/s00/e0000/4S/r00000/taupair/' +
    'mdst/sub00/mdst_000001_prod00006108_task00000001.root',
    "MC11_mixedBGx0": '/ghi/fs01/belle2/bdata/MC/release-02-00-01/DB00000411/MC11/prod00005215/s00/e0000/4S/r00000/mixed/' +
    'mdst/sub00/mdst_000985_prod00005215_task00000990.root',

}


MC10_types = {
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
}


MC9_types = {
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
}

for skim in all_skims:
    for dtype, input_file in MC12_types.items():
        print(skim)
        print(dtype)
        script = f"../combined/{skim}_Skim_Standalone.py"
        log_file = f"{skim}_{dtype}.out"
        err_file = f"{skim}_{dtype}.err"
        output_file = f"combinedFiles/{skim}_{dtype}.udst.root"

        print(f"Running {script} on {input_file} to {output_file}")
        subprocess.run(["bsub", "-q", "l", "-oo", log_file, "-e", err_file, "basf2", script,
                        "-o", output_file, "-n", "10000", "-i", input_file])
