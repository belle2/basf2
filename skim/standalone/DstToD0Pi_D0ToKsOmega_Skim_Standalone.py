#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#

# G. Casarosa, 7/Oct/2016
#
######################################################

from ROOT import Belle2
import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK, stdE, stdMu
from stdV0s import *
from stdPi0s import *
import skimExpertFunctions as expert
gb2_setuprel = 'release-04-00-00'

skimCode = expert.encodeSkimName('DstToD0Pi_D0ToKsOmega')

c2bn2path = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=c2bn2path)

stdPi('loose', path=c2bn2path)
stdK('loose', path=c2bn2path)
stdE('loose', path=c2bn2path)
stdMu('loose', path=c2bn2path)
stdPi('all', path=c2bn2path)
stdK('all', path=c2bn2path)
stdE('all', path=c2bn2path)
stdMu('all', path=c2bn2path)
stdKshorts(path=c2bn2path)
loadStdSkimPi0(path=c2bn2path)

from skim.charm import DstToD0PiD0ToKsOmega
DstList = DstToD0PiD0ToKsOmega(c2bn2path)
expert.skimOutputUdst(skimCode, DstList, path=c2bn2path)

ma.summaryOfLists(DstList, path=c2bn2path)

expert.setSkimLogging(path=c2bn2path)
b2.process(c2bn2path)

print(statistics)
