#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#######################################################
#
# Resonance rediscovery skim
# Y. Kato, Mar/2018
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdMu, stdPi, stdPr
from stdPi0s import stdPi0s
import skimExpertFunctions as expert

b2.set_log_level(b2.LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

syspath = b2.Path()
fileList = expert.get_test_file("mixedBGx1", "MC12")

ma.inputMdstList('default', fileList, path=syspath)

argvs = sys.argv
argc = len(argvs)

stdPi('loose', path=syspath)
stdK('loose', path=syspath)
stdMu('loose', path=syspath)
stdPr('loose', path=syspath)
stdPi0s('looseFit', path=syspath)


skimCode = expert.encodeSkimName('Resonance')

from skim.systematics import *
ResonanceList = ResonanceList(path=syspath)

expert.skimOutputUdst(skimCode, ResonanceList, path=syspath)
ma.summaryOfLists(ResonanceList, path=syspath)

expert.setSkimLogging(path=syspath)
b2.process(syspath)

print(statistics)
