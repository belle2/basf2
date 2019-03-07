#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdV0s import *
from stdPi0s import *
from skimExpertFunctions import *
gb2_setuprel = 'release-03-00-03'
set_log_level(LogLevel.INFO)
import sys
import os
import glob
skimCode = encodeSkimName('Charm3BodyHadronic1')

c3bh1path = Path()

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=c3bh1path)

loadStdSkimPhoton(path=c3bh1path)
loadStdSkimPi0(path=c3bh1path)
stdPi('loose', path=c3bh1path)
stdK('loose', path=c3bh1path)
stdE('loose', path=c3bh1path)
stdMu('loose', path=c3bh1path)
stdPi('all', path=c3bh1path)
stdK('all', path=c3bh1path)
stdE('all', path=c3bh1path)
stdMu('all', path=c3bh1path)
stdKshorts(path=c3bh1path)

from skim.charm import DstToD0PiD0ToHpHmPi0
DstToD0PiD0ToHpHmPi0List = DstToD0PiD0ToHpHmPi0(path=c3bh1path)
skimOutputUdst(skimCode, DstToD0PiD0ToHpHmPi0List, path=c3bh1path)

summaryOfLists(DstToD0PiD0ToHpHmPi0List, path=c3bh1path)


setSkimLogging(path=c3bh1path)
process(c3bh1path)

print(statistics)
