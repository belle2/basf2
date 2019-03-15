#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Tau generic skims
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from skimExpertFunctions import *

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-03-00-03'

skimCode = encodeSkimName('TauGeneric')
import sys
import os
import glob
fileList = get_test_file("mixedBGx1", "MC11")

inputMdstList('default', fileList)

stdPi('all')
stdPhotons('all')

# Tau Skim
from skim.taupair import *
tauList = TauList()

skimOutputUdst(skimCode, tauList)
summaryOfLists(tauList)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
