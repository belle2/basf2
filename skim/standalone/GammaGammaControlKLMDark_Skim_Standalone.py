#!/usr/bin/env python3
# -*- coding: utf-8 -*-

__authors__ = "Sam Cunliffe", "Miho Wakai"

import basf2 as b2
import modularAnalysis as ma
from stdPhotons import stdPhotons
import skimExpertFunctions as expert

# create a path
darkskimpath = b2.Path()

# test input file
fileList = expert.get_test_file("MC13_mixedBGx1")
ma.inputMdstList('default', fileList, path=darkskimpath)
stdPhotons('all', path=darkskimpath)

# run the skim
from skim.dark import GammaGammaControlKLMDarkList
darklist = GammaGammaControlKLMDarkList(path=darkskimpath)
# this skim list can be prescaled with trigger-like prescale arguments
# darklist = GammaGammaControlKLMDarkList(path=darkskimpath,
#                                         prescale_high=10, prescale_low=1)
skimCode = expert.encodeSkimName('GammaGammaControlKLMDark')
print("Gamma gamma control channel skim:", skimCode)
expert.skimOutputUdst(skimCode, darklist, path=darkskimpath)
ma.summaryOfLists(darklist, path=darkskimpath)

# suppress noisy modules, and then process
expert.setSkimLogging(path=darkskimpath)
b2.process(darkskimpath)

# print out the summary
print(b2.statistics)