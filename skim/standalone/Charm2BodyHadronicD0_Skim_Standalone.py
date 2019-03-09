
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
gb2_setuprel = 'release-03-00-03'
skimCode = encodeSkimName('Charm2BodyHadronicD0')

# create a new path for each WG
c2bhdpath = Path()
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=c2bhdpath)

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
