#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Prepare all skims at once
# P. Urquijo, 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdV0s import *
from stdCharm import *
from stdLightMesons import *
from stdDiLeptons import *
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


from BtoDh_hh_List import *
loadD0bar()
BtoDhhhList = BsigToDhTohhList()
add_skim('BtoDh_hh', BtoDhhhList)


# B- to D(->Kshh)h- Skim
from BtoDh_Kshh_List import *
loadDkshh()
BtoDhKshhList = BsigToDhToKshhList()
add_skim('BtoDh_Kshh', BtoDhKshhList)


from BtoDh_Kspi0_List import *
loadDkspi0()
BtoDhKspi0List = BsigToDhToKspi0List()
add_skim('BtoDh_Kspi0', BtoDhKspi0List)

from BtoDh_Kspipipi0_List import *
loadDkspipipi0()
BtoDhKspipipi0List = BsigToDhToKspipipi0List()
add_skim('BtoDh_Kspipipi0', BtoDhKspipipi0List)


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
