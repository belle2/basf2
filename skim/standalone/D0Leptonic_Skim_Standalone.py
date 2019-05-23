#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu, stdPi
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *

gb2_setuprel = 'release-03-01-01'  # the latest version
set_log_level(LogLevel.INFO)

import os
import sys
import glob


# please adjust the skimCode, fileList and slow pi crieria according to your need.


# skimCode = encodeSkimName('D0Leptonic')  #'D0Leptonic' is not included in the skimcode list in release-03-01-01
skimCode = "pseudoSkimCode"

CharmLeptonicPath = Path()

# fileList = get_test_file("mixedBGx1", "MC11") # Can I use both BGx0 and BGx1?


fileList = get_test_file("mixedBGx1", "MC10")
# I'd like to use ccbar.


# Can I use both BGx0 and BGx1? #'MC11' is not included in the testfile list in release-03-01-01
inputMdstList('default', fileList, path=CharmLeptonicPath)


loadStdSkimPi0(path=CharmLeptonicPath)
loadStdSkimPhoton(path=CharmLeptonicPath)
stdMu('all', path=CharmLeptonicPath)  # all? loose?
stdE('all', path=CharmLeptonicPath)
stdPi('all', path=CharmLeptonicPath)
cutAndCopyList('pi+:spi', 'pi+:all', 'pionID > 0.4', path=CharmLeptonicPath)  # the criteria of slow pi need to be decided

# from skim.charm import DstToD0LeptonicDecay #if it defined.
from D0Leptonic_List import DstToD0LeptonicDecay  # if it defined.
D0LeptonicDecay = DstToD0LeptonicDecay(CharmLeptonicPath)
skimOutputUdst(skimCode, D0LeptonicDecay, path=CharmLeptonicPath)
summaryOfLists(D0LeptonicDecay, path=CharmLeptonicPath)

setSkimLogging(path=CharmLeptonicPath)
process(CharmLeptonicPath)

print(statistics)
