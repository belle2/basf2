#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert

gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)

skimCode = expert.encodeSkimName('DstToD0Pi_D0ToHpJm')

c2bhpath = b2.Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

ma.inputMdstList('MC9', fileList, path=c2bhpath)

# stdPi('loose', path=c2bhpath)
# stdK('loose', path=c2bhpath)
# stdPi('all', path=c2bhpath)
# stdK('all', path=c2bhpath)
# stdE('all', path=c2bhpath)

from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(c2bhpath)

expert.skimOutputUdst(skimCode, DstToD0PiD0ToHpJmList, path=c2bhpath)
ma.summaryOfLists(DstToD0PiD0ToHpJmList, path=c2bhpath)


expert.setSkimLogging(path=c2bhpath)
b2.process(c2bhpath)

print(statistics)
