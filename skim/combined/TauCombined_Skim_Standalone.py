#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Run TauGeneric, TauLFV, and TauThrust skims together
"""

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi, stdPr
from stdPhotons import stdPhotons
from skim.standardlists.lightmesons import loadStdAllRho0, loadStdAllKstar0, loadStdAllPhi, loadStdAllF_0
import skimExpertFunctions as expert

skimpath = b2.Path()
fileList = expert.get_test_file('MC13_mixedBGx1')
ma.inputMdstList('default', fileList, path=skimpath)

stdE('all', path=skimpath)
stdMu('all', path=skimpath)
stdPi('all', path=skimpath)
stdK('all', path=skimpath)
stdPr('all', path=skimpath)
stdPhotons('all', path=skimpath)
loadStdAllRho0(path=taulfvskim)
loadStdAllKstar0(path=taulfvskim)
loadStdAllPhi(path=taulfvskim)
loadStdAllF_0(path=taulfvskim)

from skim.taupair import TauLFVList, TauList, TauThrustList
expert.add_skim('TauLFV', TauLFVList(1, path=skimpath), path=skimpath)
expert.add_skim('TauGeneric', TauList(path=skimpath), path=skimpath)
expert.add_skim('TauThrust', TauThrustList(path=skimpath), path=skimpath)

expert.setSkimLogging(path=skimpath)
b2.process(path=skimpath)

# print out the summary
print(b2.statistics)
