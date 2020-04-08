#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Run TauGeneric, TauLFV, and TauThrust skims together
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons
import skimExpertFunctions as expert

skimpath = b2.Path()
fileList = expert.get_test_file('MC12_mixedBGx1')
ma.inputMdstList('default', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
stdPr('all', path=skimpath)
stdPhotons('all', path=skimpath)

from skim.taupair import TauLFVList, TauList, TauThrustList
expert.add_skim('TauLFV', TauLFVList(1, path=skimpath), path=skimpath)
expert.add_skim('TauGeneric', TauList(path=skimpath), path=skimpath)
expert.add_skim('TauThrust', TauThrustList(path=skimpath), path=skimpath)

expert.setSkimLogging(path=skimpath)
b2.process(path=skimpath)

# print out the summary
print(b2.statistics)
