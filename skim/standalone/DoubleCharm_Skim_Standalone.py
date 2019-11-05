#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Double charm skims
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file
set_log_level(LogLevel.INFO)


gb2_setuprel = 'release-04-00-00'
import os
import sys
import glob
skimCode = encodeSkimName('DoubleCharm')
fileList = get_test_file("mixedBGx1_MC12")

path = Path()
inputMdstList('default', fileList, path=path)
stdPi('all', path=path)
stdPi('loose', path=path)
stdK('loose', path=path)
stdKshorts(path=path)
loadStdSkimPi0(path=path)
loadStdSkimPhoton(path=path)
stdPi0s('loose', path=path)
stdPhotons('loose', path=path)
loadStdD0(path=path)
loadStdDplus(path=path)
loadStdDstar0(path=path)
loadStdDstarPlus(path=path)

# Double Charm Skim
from skim.btocharm import *
DCList = DoubleCharmList(path=path)
skimOutputUdst(skimCode, DCList, path=path)
summaryOfLists(DCList, path=path)
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
