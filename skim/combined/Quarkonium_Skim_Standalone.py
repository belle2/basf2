#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Bottomonium skims
# S. Spataro & Sen Jia, 14/Nov/2018
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdPhotons import stdPhotons
from stdCharged import stdE, stdK, stdMu, stdPi
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

# create a new path
skimpath = b2.Path()


fileList = expert.get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=skimpath)

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
expert.add_skim("ISRpipicc", ISRpipiccList(path=skimpath), skimpath)
# Bottomonium Etab Skim: 15420100
from skim.quarkonium import EtabList
expert.add_skim("BottomoniumEtabExclusive", EtabList(path=skimpath), path=skimpath)
# Bottomonium Upsilon Skim: 15440100
from skim.quarkonium import UpsilonList
expert.add_skim('BottomoniumUpsilon', UpsilonList(path=skimpath), path=skimpath)


expert.setSkimLogging(path=skimpath)
b2.process(skimpath)

# print out the summary
print(statistics)
