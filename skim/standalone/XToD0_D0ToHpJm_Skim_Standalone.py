
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)

gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('XToD0_D0ToHpJm')

# create a new path for each WG
c2bhdpath = b2.Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
ma.inputMdstList('MC9', fileList, path=c2bhdpath)

# stdPi('loose', path=c2bhdpath)
# stdK('loose', path=c2bhdpath)
# stdPi('all', path=c2bhdpath)
# stdK('all', path=c2bhdpath)

from skim.charm import D0ToHpJm
D0ToHpJmList = D0ToHpJm(c2bhdpath)
expert.skimOutputUdst(skimCode, D0ToHpJmList, path=c2bhdpath)
ma.summaryOfLists(D0ToHpJmList, path=c2bhdpath)


expert.setSkimLogging(path=c2bhdpath)
b2.process(c2bhdpath)

print(statistics)
