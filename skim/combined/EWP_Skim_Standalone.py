#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from stdPhotons import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging, get_test_file
gb2_setuprel = 'release-03-02-00'


fileList = get_test_file("mixedBGx1", "MC12")

xggpath = Path()


inputMdstList('default', fileList, path=xggpath)
stdPi0s('loose', path=xggpath)
stdPhotons('tight', path=xggpath)  # also builds loose list
loadStdSkimPhoton(path=xggpath)
loadStdSkimPi0(path=xggpath)
stdPi('loose', path=xggpath)
stdK('loose', path=xggpath)
stdK('95eff', path=xggpath)
stdPi('95eff', path=xggpath)
stdKshorts(path=xggpath)
loadStdLightMesons(path=xggpath)

cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1', path=xggpath)
reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580', path=xggpath)
# EWP Skim
from skim.ewp import B2XgammaList
add_skim('BtoXgamma', B2XgammaList(path=xggpath), xggpath)


setSkimLogging(xggpath)
process(xggpath)


xllpath = Path()
inputMdstList('default', fileList, path=xllpath)
loadStdSkimPi0(path=xllpath)
loadStdSkimPhoton(path=xllpath)
stdPi0s('loose', path=xllpath)
stdPhotons('loose', path=xllpath)
stdK('95eff', path=xllpath)
stdPi('95eff', path=xllpath)
stdE('95eff', path=xllpath)
stdMu('95eff', path=xllpath)
stdK('loose', path=xllpath)
stdPi('loose', path=xllpath)
stdKshorts(path=xllpath)
loadStdLightMesons(path=xllpath)

cutAndCopyList('gamma:ewp', 'gamma:loose', 'E > 0.1', path=xllpath)
reconstructDecay('eta:ewp -> gamma:ewp gamma:ewp', '0.505 < M < 0.580', path=xllpath)
# EWP Skim
from skim.ewp import B2XllList
add_skim('BtoXll', B2XllList(path=xllpath), path=xllpath)

setSkimLogging(xllpath)
process(xllpath)


# print out the summary
print(statistics)
