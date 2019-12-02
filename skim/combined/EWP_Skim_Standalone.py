#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP combined skim standalone steering script
#
# Trevor Shillington August 2019
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
import skimExpertFunctions as expert

gb2_setuprel = 'release-04-00-00'
fileList = expert.get_test_file("mixedBGx1", "MC12")
path = b2.Path()

ma.inputMdstList('default', fileList, path=path)

# import standard lists
stdPhotons('loose', path=path)
stdPhotons('all', path=path)
stdPi('all', path=path)
stdE('loose', path=path)
stdMu('loose', path=path)


# BtoXgamma
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList(path=path)
expert.add_skim("BtoXgamma", XgammaList, path=path)

# BtoXll
from skim.ewp import B2XllList
XllList = B2XllList(path=path)
expert.add_skim("BtoXll", XllList, path=path)

# BtoXll_LFV
from skim.ewp import B2XllListLFV
XllListLFV = B2XllListLFV(path=path)
expert.add_skim("BtoXll_LFV", XllListLFV, path=path)


# process
expert.setSkimLogging(path=path)
b2.process(path=path)

# print out the summary
print(statistics)
