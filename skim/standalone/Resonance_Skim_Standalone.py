#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#######################################################
#
# Resonance rediscovery skim
# Y. Kato, Mar/2018
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdMu, stdPr
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import encodeSkimName, setSkimLogging, get_test_file

set_log_level(LogLevel.INFO)
gb2_setuprel = 'release-04-00-00'

syspath = Path()
fileList = get_test_file("mixedBGx1", "MC12")

inputMdstList('default', fileList, path=syspath)

argvs = sys.argv
argc = len(argvs)

stdPi('loose', path=syspath)
stdK('loose', path=syspath)
stdMu('loose', path=syspath)
stdPr('loose', path=syspath)
stdPi0s('looseFit', path=syspath)


skimCode = encodeSkimName('Resonance')

from skim.systematics import *
ResonanceList = ResonanceList(path=syspath)

skimOutputUdst(skimCode, ResonanceList, path=syspath)
summaryOfLists(ResonanceList, path=syspath)

setSkimLogging(path=syspath)
process(syspath)

print(statistics)
