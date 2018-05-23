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
    ['/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000001/s00/e0001/4S/r00001/mixed/sub00/' +
     'mdst_000001_prod00000001_task00000001.root'

     ]


inputMdstList('default', fileList)


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
    getOutputLFN(label)
    skimOutputUdst(label, lists)
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


for module in analysis_main.modules():
    if module.type() == "ParticleLoader":
        module.set_log_level(LogLevel.ERROR)
process(analysis_main)
process(analysis_main)

# print out the summary
print(statistics)
