
#: the total number if input files for each campaign
total_input_files = [
    ('MC12_mixedBGx1', 1070),
    ('MC12_chargedBGx1', 1135),
    ('MC12_uubarBGx1', 3210),
    ('MC12_ddbarBGx1', 805),
    ('MC12_ssbarBGx1', 770),
    ('MC12_ccbarBGx1', 2660),
    ('MC12_taupairBGx1', 1840),
    ('MC12_mixedBGx0', 270),
    ('MC12_chargedBGx0', 285),
    ('MC12_uubarBGx0', 805),
    ('MC12_ddbarBGx0', 205),
    ('MC12_ssbarBGx0', 195),
    ('MC12_ccbarBGx0', 665),
    ('MC12_taupairBGx0', 460),


    ('MC13_mixedBGx1', 2550),
    ('MC13_chargedBGx1', 2700),
    ('MC13_uubarBGx1', 8030),
    ('MC13_ddbarBGx1', 2010),
    ('MC13_ssbarBGx1', 1920),
    ('MC13_ccbarBGx1', 6650),
    ('MC13_taupairBGx1', 4600),

    ('MC13_ggBGx1', 25201),
    ('MC13_eeBGx1', -1),  # TODO
    ('MC13_mumuBGx1', 2297),
    ('MC13_eeeeBGx1', 36843),
    ('MC13_eemumuBGx1', 15096),

    ('MC13_mixedBGx0', 510),
    ('MC13_chargedBGx0', 540),
    ('MC13_uubarBGx0', 1606),
    ('MC13_ddbarBGx0', 402),
    ('MC13_ssbarBGx0', 384),
    ('MC13_ccbarBGx0', 1330),
    ('MC13_taupairBGx0', 920),

]

# Average number of events in test files
nEventsPerFile = {

    # TODO: get estimates for these
    # 'proc10_exp7': ,
    # 'proc10_exp8': ,

    'proc9_exp3': 3456,
    'proc9_exp7': 3691,
    'proc9_exp8': 7846,
    'bucket7_exp8': 11242,

    'MC13_mixedBGx1': 200000,
    'MC13_chargedBGx1': 200000,
    'MC13_uubarBGx1': 197609,
    'MC13_ddbarBGx1':  197673,
    'MC13_ssbarBGx1': 189320,
    'MC13_ccbarBGx1': 195621,
    'MC13_taupairBGx1': 200000,

    'MC13_ggBGx1': 9390,
    'MC13_eeBGx1': -1,  # TODO
    'MC13_mumuBGx1': 10000,
    'MC13_eeeeBGx1': 10000,
    'MC13_eemumuBGx1': 10000,

    'MC12_mixedBGx1': 200000,
    'MC12_chargedBGx1': 200000,
    'MC12_uubarBGx1': 200000,
    'MC12_ddbarBGx1': 200000,
    'MC12_ssbarBGx1': 200000,
    'MC12_ccbarBGx1': 200000,
    'MC12_taupairBGx1': 200000,
    'MC12_mixedBGx0': 200000,
    'MC12_chargedBGx0': 200000,
    'MC12_uubarBGx0': 200000,
    'MC12_ddbarBGx0': 200000,
    'MC12_ssbarBGx0': 200000,
    'MC12_ccbarBGx0': 200000,
    'MC12_taupairBGx0': 200000,
}

#: location of test files on kekcc
kekcc_locations = [

    # data
    ('proc10_exp7', '/group/belle2/dataprod/Data/release-04-01-00/DB00000748/proc10/e0007/4S/GoodRuns/r03491/all/' +
     'mdst/sub00/mdst.physics.0007.03491.HLT1.f00001.root'),
    ('proc10_exp8', '/group/belle2/dataprod/Data/release-04-01-00/DB00000748/proc10/e0008/4S/GoodRuns/r01291/all/' +
     'mdst/sub00/mdst.physics.0008.01291.HLT2.f00010.root'),
    ('proc9_exp3', '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/e0003/4S/r05241/skim/hlt_hadron/' +
     'mdst/sub00/mdst.physics.0003.05241.HLT0.hlt_hadron.f00000.root'),
    ('proc9_exp7', '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/e0007/4S/GoodRuns/r03822/skim/hlt_hadron/' +
     'mdst/sub00/mdst.physics.0007.03822.HLT0.hlt_hadron.f00000.root'),
    ('proc9_exp8', '/group/belle2/dataprod/Data/release-03-02-02/DB00000654/proc9/e0008/4S/GoodRuns/r01527/skim/hlt_hadron/' +
     'mdst/sub00/mdst.physics.0008.01527.HLT4.hlt_hadron.f00000.root'),
    ('bucket7_exp8', '/group/belle2/dataprod/Data/release-03-02-04/DB00000677/bucket7/e0008/4S/GoodRuns/r03123/skim/hlt_hadron/' +
     'mdst/sub00/mdst.physics.0008.03123.HLT4.hlt_hadron.f00000.root'),

    # generic MC13 BGx1
    ("MC13_ccbarBGx1", "/group/belle2/dataprod/MC/SkimTraining/ccbar_BGx1.mdst_000001_prod00009437_task10020000001.root"),
    ("MC13_chargedBGx1", "/group/belle2/dataprod/MC/SkimTraining/charged_BGx1.mdst_000001_prod00009435_task10020000001.root"),
    ("MC13_ddbarBGx1", "/group/belle2/dataprod/MC/SkimTraining/ddbar_BGx1.mdst_000001_prod00009438_task10020000001.root"),
    ("MC13_mixedBGx1", "/group/belle2/dataprod/MC/SkimTraining/mixed_BGx1.mdst_000001_prod00009434_task10020000001.root"),
    ("MC13_ssbarBGx1", "/group/belle2/dataprod/MC/SkimTraining/ssbar_BGx1.mdst_000001_prod00009439_task10020000001.root"),
    ("MC13_taupairBGx1", "/group/belle2/dataprod/MC/SkimTraining/taupair_BGx1.mdst_000001_prod00009441_task10020000001.root"),
    ("MC13_uubarBGx1", "/group/belle2/dataprod/MC/SkimTraining/uubar_BGx1.mdst_000001_prod00009436_task10020000001.root"),

    # low multiplicity MC13
    # TODO: replace these with a more permanent location for test samples
    ('MC13_ggBGx1', '/home/belle2/pgrace/TemporarySkimSamples/3700001000_eph3_0.root'),
    # ('MC13_eeBGx1', 'FIXME')
    ('MC13_mumuBGx1', '/home/belle2/pgrace/TemporarySkimSamples/mumu_eph3_0.root'),
    ('MC13_eeeeBGx1', '/home/belle2/pgrace/TemporarySkimSamples/eeee_eph3_0.root'),
    ('MC13_eemumuBGx1', '/home/belle2/pgrace/TemporarySkimSamples/eemumu_eph3_0.root'),

    # generic MC13 BGx0
    ("MC13_ccbarBGx0", "/group/belle2/dataprod/MC/SkimTraining/ccbar_BGx0.mdst_000001_prod00009559_task10020000001.root"),
    ("MC13_chargedBGx0", "/group/belle2/dataprod/MC/SkimTraining/charged_BGx0.mdst_000001_prod00009551_task10020000001.root"),
    ("MC13_ddbarBGx0", "/group/belle2/dataprod/MC/SkimTraining/ddbar_BGx0.mdst_000001_prod00009555_task10020000001.root"),
    ("MC13_mixedBGx0", "/group/belle2/dataprod/MC/SkimTraining/mixed_BGx0.mdst_000001_prod00009545_task10020000001.root"),
    ("MC13_ssbarBGx0", "/group/belle2/dataprod/MC/SkimTraining/ssbar_BGx0.mdst_000023_prod00009557_task10020000023.root"),
    ("MC13_uubarBGx0", "/group/belle2/dataprod/MC/SkimTraining/uubar_BGx0.mdst_000003_prod00009553_task10020000003.root"),
    ("MC13_taupairBGx0", "/group/belle2/dataprod/MC/SkimTraining/taupair_BGx0.mdst_000001_prod00009561_task10020000001.root"),

    # generic MC12 BGx1
    ('MC12_mixedBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S/r00000/mixed/' +
     'mdst/sub00/mdst_000141_prod00007392_task10020000141.root'),
    ('MC12_chargedBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007394/s00/e1003/4S/r00000/charged/' +
     'mdst/sub00/mdst_000225_prod00007394_task10020000225.root'),
    ('MC12_ccbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007426/s00/e1003/4S/r00000/ccbar/' +
     'mdst/sub00/mdst_000262_prod00007426_task10020000262.root'),
    ('MC12_uubarBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007396/s00/e1003/4S/r00000/uubar/' +
     'mdst/sub00/mdst_000621_prod00007396_task10020000627.root'),
    ('MC12_ddbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007398/s00/e1003/4S/r00000/ddbar/' +
     'mdst/sub00/mdst_000068_prod00007398_task10020000068.root'),
    ('MC12_ssbarBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007400/s00/e1003/4S/r00000/ssbar/' +
     'mdst/sub00/mdst_000141_prod00007400_task10020000142.root'),
    ('MC12_taupairBGx1', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007404/s00/e1003/4S/r00000/taupair/' +
     'mdst/sub00/mdst_000171_prod00007404_task10020000172.root'),

    # generic MC12 BGx0
    ('MC12_mixedBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007393/s00/e1003/4S/r00000/mixed/' +
     'mdst/sub00/mdst_000018_prod00007393_task10020000018.root'),
    ('MC12_chargedBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007395/s00/e1003/4S/r00000/charged/' +
     'mdst/sub00/mdst_000007_prod00007395_task10020000007.root'),
    ('MC12_ccbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007403/s00/e1003/4S/r00000/ccbar/' +
     'mdst/sub00/mdst_000053_prod00007403_task10020000053.root'),
    ('MC12_uubarBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007397/s00/e1003/4S/r00000/uubar/' +
     'mdst/sub00/mdst_000074_prod00007397_task10020000074.root'),
    ('MC12_ddbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007399/s00/e1003/4S/r00000/ddbar/' +
     'mdst/sub00/mdst_000006_prod00007399_task10020000006.root'),
    ('MC12_ssbarBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007401/s00/e1003/4S/r00000/ssbar/' +
     'mdst/sub00/mdst_000025_prod00007401_task10020000025.root'),
    ('MC12_taupairBGx0', '/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007405/s00/e1003/4S/r00000/taupair/' +
     'mdst/sub00/mdst_000034_prod00007405_task10020000034.root'),

    # low multiplicity MC12
    ('MC12_eeeeBGx0', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007515_task10020000001.root'),
    ('MC12_eeeeBGx1', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007464_task10020000001.root'),
    ('MC12_eemumuBGx0', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007516_task10020000001.root'),
    # ('MC12_eemumuBGx0','/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007465_task10020000001.root'),
    ('MC12_mumuBGx0', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007517_task10020000001.root'),
    ('MC12_mumuBGx1', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007466_task10020000001.root'),
    ('MC12_eeBGx0', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007519_task10020000001.root'),
    ('MC12_eeBGx1', '/ghi/fs01/belle2/bdata/group/skim/sub00/mdst_000001_prod00007468_task10020000001.root'),
]
