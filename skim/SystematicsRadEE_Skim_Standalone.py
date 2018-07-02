#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Systematics Skims for radiative electron pairs
# Sam Cunliffe (sam.cunliffe@desy.de), 2018
#
#######################################################

from basf2 import process, statistics
from modularAnalysis import analysis_main, inputMdstList, \
        skimOutputUdst, summaryOfLists
from stdCharged import loadStdCharged
from skimExpertFunctions import encodeSkimName, setSkimLogging

gb2_setuprel = 'release-02-00-00'

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002314/e0000/4S/r00000/mumu_ecldigits/sub00/' +
    'mdst_000001_prod00002314_task00000001.root']

inputMdstList('MC9', fileList)
loadStdCharged()

from SystematicsRadEE_List import SystematicsRadEEList
radeelist = SystematicsRadEEList()
skimcode = encodeSkimName('SystematicsRadEE')
skimOutputUdst(skimcode, radeelist)
summaryOfLists(radeelist)

setSkimLogging()
process(analysis_main)
print(statistics)
