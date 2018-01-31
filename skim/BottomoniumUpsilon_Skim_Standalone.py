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
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *
set_log_level(LogLevel.ERROR)
gb2_setuprel = 'release-01-00-00'
import sys
import os
import glob

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
    'mdst_001724_prod00000962_task00001729.root']

inputMdstList('default', fileList)
stdPhotons('loose')

# Bottomonium Skim
from BottomoniumUpsilon_List import *
YList = UpsilonList()
skimOutputUdst('BottomoniumUpsilon', YList)

summaryOfLists(YList)


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)

# print out the summary
print(statistics)
