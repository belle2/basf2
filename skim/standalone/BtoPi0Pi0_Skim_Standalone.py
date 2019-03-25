# !/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# B0 -> pi0 pi0
#
# K. Smith
#
#######################################################

from basf2 import *
from ROOT import Belle2
from modularAnalysis import *
from stdPi0s import *

from skimExpertFunctions import *
set_log_level(LogLevel.INFO)

gb2_setuprel = "release-03-00-03"

import os
import sys
import glob
skimCode = encodeSkimName('BtoPi0Pi0')

path = Path()
fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=path)

# load particle lists
loadStdSkimPi0(path=path)

# Had Skim
from skim.btocharmless import BtoPi0Pi0List
Pi0Pi0List = BtoPi0Pi0List(path=path)
skimOutputUdst(skimCode, Pi0Pi0List, path=path)

summaryOfLists(Pi0Pi0List, path=path)


setSkimLogging()
process(path)

# print out the summary
print(statistics)
