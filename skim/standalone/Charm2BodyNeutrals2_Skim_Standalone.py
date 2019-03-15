#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#

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

skimCode = encodeSkimName('Charm2BodyNeutrals2')

c2bn2path = Path()

fileList = get_test_file("mixedBGx1", "MC11")
inputMdstList('default', fileList, path=c2bn2path)

stdPi('loose', path=c2bn2path)
stdK('loose', path=c2bn2path)
stdE('loose', path=c2bn2path)
stdMu('loose', path=c2bn2path)
stdPi('all', path=c2bn2path)
stdK('all', path=c2bn2path)
stdE('all', path=c2bn2path)
stdMu('all', path=c2bn2path)
stdKshorts(path=c2bn2path)
mergedKshorts(path=c2bn2path)
loadStdSkimPi0(path=c2bn2path)

from skim.charm import DstToD0PiD0ToKsOmega
DstList = DstToD0PiD0ToKsOmega(c2bn2path)
skimOutputUdst(skimCode, DstList, path=c2bn2path)

summaryOfLists(DstList, path=c2bn2path)

setSkimLogging(path=c2bn2path)
process(c2bn2path)

print(statistics)
