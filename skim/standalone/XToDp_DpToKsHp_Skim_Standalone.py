
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdV0s import mergedKshorts
from skimExpertFunctions import *
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--data',
                    help='Provide this flag if running on data.',
                    action='store_true', default=False)
args = parser.parse_args()

if args.data:
    use_central_database("data_reprocessing_prompt_bucket6")

set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-03-02-02'

import os
import sys
import glob
skimCode = encodeSkimName('XToDp_DpToKsHp')

# create a new path for each WG
ckshppath = Path()

fileList = ['/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S/r00000/mixed/' +
            'mdst/sub00/mdst_000141_prod00007392_task10020000141.root']

inputMdstList('default', fileList, path=ckshppath)


mergedKshorts(path=ckshppath)
stdPi('loose', path=ckshppath)
stdK('loose', path=ckshppath)
# stdPi('all', path=ckshppath)
# stdK('all', path=ckshppath)

from skim.charm import DpToKsHp
DpToKsHpList = DpToKsHp(ckshppath)
skimOutputUdst(skimCode, DpToKsHpList, path=ckshppath)
summaryOfLists(DpToKsHpList, path=ckshppath)


setSkimLogging(path=ckshppath)
process(ckshppath)

print(statistics)
