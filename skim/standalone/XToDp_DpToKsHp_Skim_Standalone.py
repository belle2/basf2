# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdV0s import mergedKshorts
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)

gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('XToDp_DpToKsHp')

# create a new path for each WG
ckshppath = b2.Path()

fileList = ['/ghi/fs01/belle2/bdata/MC/release-03-01-00/DB00000547/MC12b/prod00007392/s00/e1003/4S/r00000/mixed/' +
            'mdst/sub00/mdst_000141_prod00007392_task10020000141.root']

ma.inputMdstList('default', fileList, path=ckshppath)


mergedKshorts(path=ckshppath)
stdPi('loose', path=ckshppath)
stdK('loose', path=ckshppath)

from skim.charm import DpToKsHp
DpToKsHpList = DpToKsHp(ckshppath)
expert.skimOutputUdst(skimCode, DpToKsHpList, path=ckshppath)
ma.summaryOfLists(DpToKsHpList, path=ckshppath)


expert.setSkimLogging(path=ckshppath)
b2.process(ckshppath)

print(b2.statistics)
