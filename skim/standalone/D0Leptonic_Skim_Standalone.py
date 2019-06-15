#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu, stdPi
from stdPi0s import *
from stdPhotons import *
from skimExpertFunctions import *

gb2_setuprel = 'release-03-01-02'  # modified to release-03-01-02 from release-03-01-01
set_log_level(LogLevel.INFO)

import os
import sys
import glob


# skimCode = encodeSkimName('D0Leptonic')  #'D0Leptonic' is not included in the skimcode list in release-03-01-01
skimCode = "D0Leptonic"

CharmLeptonicPath = Path()


modes = ['mixed', 'charged', 'uubar', 'ddbar', 'ssbar', 'ccbar', 'taupair']
backgrounds = ['BGx0', 'BGx1']
mcNumber = "MC12"
fileLists = []


# Appending all combinations to input file list.
for mode in modes:
    for background in backgrounds:
        fileLists.append(get_test_file(mode + background, mcNumber))


# Can I use both BGx0 and BGx1? #'MC11' is not included in the testfile list in release-03-01-01
inputMdstList('default', fileLists, path=CharmLeptonicPath)


loadStdSkimPi0(path=CharmLeptonicPath)
loadStdSkimPhoton(path=CharmLeptonicPath)
stdMu('loose', path=CharmLeptonicPath)  # I choose loose cut.
stdE('loose', path=CharmLeptonicPath)
stdPi('loose', path=CharmLeptonicPath)
copyList('pi+:spi', 'pi+:loose', path=CharmLeptonicPath)  # the criteria of spi is removed.


from skim.charm import DstToD0Leptonic
D0LeptonicList = DstToD0Leptonic(CharmLeptonicPath)
skimOutputUdst(skimCode, D0LeptonicList, path=CharmLeptonicPath)
summaryOfLists(D0LeptonicList, path=CharmLeptonicPath)

setSkimLogging(path=CharmLeptonicPath)
process(CharmLeptonicPath)

print(statistics)
