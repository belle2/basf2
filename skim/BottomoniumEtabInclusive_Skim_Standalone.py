#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro, 25/Jul/2016
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import *
set_log_level(LogLevel.ERROR)
gb2_setuprel = 'build-2017-10-16'
import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
    'mdst_001724_prod00000962_task00001729.root']

inputMdstList('default', fileList)
loadStdCharged()
loadStdSkimPhoton()

# Bottomonium Skim
from BottomoniumEtabInclusive_List import *
EtabList = EtabInclusiveList()
skimOutputUdst('BottomoniumEtabInclusive', EtabList)
summaryOfLists(EtabList)
process(analysis_main)

# print out the summary
print(statistics)
