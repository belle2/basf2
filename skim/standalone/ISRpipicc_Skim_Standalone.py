#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charmonium skims
# Sen Jia, 27/Sep/2018
# ISRpipicc_List skim
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'
b2.set_log_level(b2.LogLevel.INFO)


# create a new path
ISRskimpath = b2.Path()

# Add default samples
fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=ISRskimpath)

# use standard final state particle lists
stdPi('loose', path=ISRskimpath)
stdK('loose', path=ISRskimpath)
stdE('loose', path=ISRskimpath)
stdMu('loose', path=ISRskimpath)
stdPi('all', path=ISRskimpath)
stdK('all', path=ISRskimpath)
stdE('all', path=ISRskimpath)
stdMu('all', path=ISRskimpath)

# importing the reconstructed events from the ISRpipicc_List file
from skim.quarkonium import ISRpipiccList
ISRpipicc = ISRpipiccList(path=ISRskimpath)

# output to Udst file
skimCode = expert.encodeSkimName('ISRpipicc')
expert.skimOutputUdst(skimCode, ISRpipicc, path=ISRskimpath)

# print out Particle List statistics
ma.summaryOfLists(ISRpipicc, path=ISRskimpath)

# output skim log information
expert.setSkimLogging(path=ISRskimpath)

# process the path
b2.process(ISRskimpath)

# print out the summary
print(b2.statistics)
