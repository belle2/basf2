#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Run TauGeneric, TauLFV, and TauThrust skims together
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdPi, stdK, stdPr, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import *
from skim.standardlists.charm import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file

skimpath = b2.Path()
fileList = get_test_file('mixedBGx1', 'MC12')
ma.inputMdstList('default', fileList, path=skimpath)

stdE('loose', path=skimpath)
stdK('loose', path=skimpath)
stdKshorts(path=skimpath)
stdMu('loose', path=skimpath)
stdPhotons('all', path=skimpath)
stdPhotons('loose', path=skimpath)
stdPi('all', path=skimpath)
stdPi('loose', path=skimpath)
stdPi0s('loose', path=skimpath)
stdPr('loose', path=skimpath)
loadStdLightMesons(path=skimpath)
loadStdSkimPi0(path=skimpath)

from skim.taupair import TauLFVList, TauList, TauThrustList
add_skim('TauLFV', TauLFVList(path=skimpath), path=skimpath)
add_skim('TauGeneric', TauList(path=skimpath), path=skimpath)
add_skim('TauThrust', TauThrustList(path=skimpath), path=skimpath)

setSkimLogging(path=skimpath)
b2.process(path=skimpath)

# print out the summary
print(b2.statistics)
