#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# P. Urquijo, 6/Jan/2015
# G. Casarosa, 7/Oct/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *
gb2_setuprel = 'release-02-00-00'
set_log_level(LogLevel.INFO)
import os
import sys
import glob
skimCode = encodeSkimName('CharmRare')
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList('default', fileList)


loadStdSkimPi0()
loadStdSkimPhoton()
stdLooseMu()
stdLooseE()
from CharmRare_List import *
CharmRareList = CharmRareList()
skimOutputUdst(skimCode, CharmRareList)
summaryOfLists(CharmRareList)

setSkimLogging()
process(analysis_main)

print(statistics)
