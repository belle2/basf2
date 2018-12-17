
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
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)

gb2_setuprel = 'release-02-00-01'

import os
import sys
import glob
skimCode = encodeSkimName('Charm2BodyHadronicD0')

# create a new path for each WG
c2bhdpath = Path()

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('MC9', fileList, path=c2bhdpath)

stdPi('loose', path=c2bhdpath)
stdK('loose', path=c2bhdpath)
stdPi('all', path=c2bhdpath)
stdK('all', path=c2bhdpath)

from skim.charm import D0ToHpJm
D0ToHpJmList = D0ToHpJm(c2bhdpath)
skimOutputUdst(skimCode, D0ToHpJmList, path=c2bhdpath)
summaryOfLists(D0ToHpJmList, path=c2bhdpath)


setSkimLogging(path=c2bhdpath)
process(c2bhdpath)

print(statistics)
