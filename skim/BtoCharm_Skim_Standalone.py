#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# All BtoCharmSkims in one _standalone
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from skimStdCharm import *
from stdLightMesons import *
from skimStdDiLeptons import *
set_log_level(LogLevel.INFO)
import os
import sys
import glob
from skimExpertFunctions import *


fileList = \
    [
        '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
        'mdst_000001_prod00002288_task00000001.root'
    ]
inputMdstList('MC9', fileList)


loadStdCharged()
stdPi0s('loose')
stdPhotons('loose')
loadStdKS()
loadStdLightMesons()
loadStdSkimPi0()
loadStdSkimPhoton()


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.

    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists)
    summaryOfLists(lists)

# B- to D(->hh)h- Skim
from skim.btocharm import BsigToDhTohhList, loadD0bar
loadD0bar()
BtoDhhhList = BsigToDhTohhList()
add_skim('BtoDh_hh', BtoDhhhList)


# B- to D(->Kshh)h- Skim
from skim.btocharm import BsigToDhToKshhList, loadDkshh
loadDkshh()
BtoDhKshhList = BsigToDhToKshhList()
add_skim('BtoDh_Kshh', BtoDhKshhList)

# B- to D(->Kspi0)h- Skim
from skim.btocharm import BsigToDhToKspi0List, loadDkspi0
loadDkspi0()
BtoDhKspi0List = BsigToDhToKspi0List()
add_skim('BtoDh_Kspi0', BtoDhKspi0List)

# B- to D(->Kspipipi0)h- Skim
from skim.btocharm import BsigToDhToKspipipi0List, loadDkspipipi0
loadDkspipipi0()
BtoDhKspipipi0List = BsigToDhToKspipipi0List()
add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List)

setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
