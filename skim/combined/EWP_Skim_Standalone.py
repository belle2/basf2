#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
#
# EWP combined skim standalone steering script
#
# Trevor Shillington August 2019
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdE, stdMu, stdPi
from stdPhotons import *
from skimExpertFunctions import *

gb2_setuprel = 'release-04-00-00'
fileList = get_test_file("mixedBGx1", "MC12")
path = Path()

inputMdstList('default', fileList, path=path)

# import standard lists
stdPhotons('loose', path=path)
stdPhotons('all', path=path)
stdPi('all', path=path)
stdE('loose', path=path)
stdMu('loose', path=path)


# BtoXgamma
from skim.ewp import B2XgammaList
XgammaList = B2XgammaList(path=path)
add_skim("BtoXgamma", XgammaList, path=path)

# BtoXll
from skim.ewp import B2XllList
XllList = B2XllList(path=path)
add_skim("BtoXll", XllList, path=path)

# BtoXll_LFV
from skim.ewp import B2XllListLFV
XllListLFV = B2XllListLFV(path=path)
add_skim("BtoXll_LFV", XllListLFV, path=path)


# process
setSkimLogging(path=path)
process(path=path)

# print out the summary
print(statistics)
