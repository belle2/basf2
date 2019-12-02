#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmSkims in one _standalone
#
######################################################

import basf2 as b2
import modularAnalysis as ma
from stdCharged import stdK, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s
from stdV0s import stdKshorts
from skim.standardlists.lightmesons import loadStdLightMesons
import skimExpertFunctions as expert

gb2_setuprel = 'release-04-00-00'

btocharmpath = b2.Path()

fileList = expert.get_test_file("mixedBGx1", "MC12")
ma.inputMdstList('default', fileList, path=btocharmpath)


stdPi('all', path=btocharmpath)
stdK('all', path=btocharmpath)
stdPi('loose', path=btocharmpath)
stdK('loose', path=btocharmpath)
stdPi0s('loose', path=btocharmpath)
stdPhotons('loose', path=btocharmpath)
stdKshorts(path=btocharmpath)
loadStdLightMesons(path=btocharmpath)
loadStdSkimPi0(path=btocharmpath)
loadStdSkimPhoton(path=btocharmpath)


# B- to D(->hh)h- Skim
from skim.btocharm import BsigToDhTohhList, loadD0bar
loadD0bar(path=btocharmpath)
BtoDhhhList = BsigToDhTohhList(path=btocharmpath)
expert.add_skim('BtoDh_hh', BtoDhhhList, path=btocharmpath)


# B- to D(->Kshh)h- Skim
from skim.btocharm import BsigToDhToKshhList, loadDkshh
loadDkshh(path=btocharmpath)
BtoDhKshhList = BsigToDhToKshhList(path=btocharmpath)
expert.add_skim('BtoDh_Kshh', BtoDhKshhList, path=btocharmpath)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import BsigToDhToKspi0List, loadDkspi0
loadDkspi0(path=btocharmpath)
BtoDhKspi0List = BsigToDhToKspi0List(path=btocharmpath)
expert.add_skim('BtoDh_Kspi0', BtoDhKspi0List, path=btocharmpath)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import BsigToDhToKspipipi0List, loadDkspipipi0
loadDkspipipi0(path=btocharmpath)
BtoDhKspipipi0List = BsigToDhToKspipipi0List(path=btocharmpath)
expert.add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List, path=btocharmpath)

expert.setSkimLogging(path=btocharmpath)
b2.process(path=btocharmpath)

# print out the summary
print(b2.statistics)
