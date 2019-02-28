#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmSkims in one _standalone
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi, stdK
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
from skimExpertFunctions import add_skim, encodeSkimName, setSkimLogging
gb2_setuprel = 'release-03-00-00'

btocharmpath = Path()
fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]
inputMdstList('MC9', fileList, path=btocharmpath)


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
add_skim('BtoDh_hh', BtoDhhhList, path=btocharmpath)


# B- to D(->Kshh)h- Skim
from skim.btocharm import BsigToDhToKshhList, loadDkshh
loadDkshh(path=btocharmpath)
BtoDhKshhList = BsigToDhToKshhList(path=btocharmpath)
add_skim('BtoDh_Kshh', BtoDhKshhList, path=btocharmpath)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import BsigToDhToKspi0List, loadDkspi0
loadDkspi0(path=btocharmpath)
BtoDhKspi0List = BsigToDhToKspi0List(path=btocharmpath)
add_skim('BtoDh_Kspi0', BtoDhKspi0List, path=btocharmpath)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import BsigToDhToKspipipi0List, loadDkspipipi0
loadDkspipipi0(path=btocharmpath)
BtoDhKspipipi0List = BsigToDhToKspipipi0List(path=btocharmpath)
add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List, path=btocharmpath)

setSkimLogging(path=btocharmpath)
process(path=btocharmpath)

# print out the summary
print(statistics)
