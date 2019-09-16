#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro & Sen Jia, 14/Nov/2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdCharged import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'

# create a new path
skimpath = Path()


fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=skimpath)

# use standard final state particle lists

stdPhotons('loose', path=skimpath)

stdPi('loose', path=skimpath)
stdK('loose', path=skimpath)
stdE('loose', path=skimpath)
stdMu('loose', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
stdE('all', path=skimpath)
stdMu('all', path=skimpath)

# ISRpipicc Skim
from skim.quarkonium import ISRpipiccList
add_skim("ISRpipicc", ISRpipiccList(path=skimpath), skimpath)
# Bottomonium Etab Skim: 15420100
from skim.quarkonium import EtabList
add_skim("BottomoniumEtabExclusive", EtabList(path=skimpath), path=skimpath)
# Bottomonium Upsilon Skim: 15440100
from skim.quarkonium import UpsilonList
add_skim('BottomoniumUpsilon', UpsilonList(path=skimpath), path=skimpath)


setSkimLogging(path=skimpath)
process(skimpath)

# print out the summary
print(statistics)
