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
from stdCharged import stdPi, stdK, stdPr, stdE, stdMu
from stdPi0s import *
from stdV0s import *
from skim.standardlists.charm import *
from skim.standardlists.lightmesons import *
from skim.standardlists.dileptons import *
set_log_level(LogLevel.INFO)

from skimExpertFunctions import *


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)


stdPi('loose')
stdK('loose')
stdPr('loose')
stdE('loose')
stdMu('loose')
stdPi('all')
stdPi0s('loose')
stdPhotons('loose')
stdKshorts()
loadStdLightMesons()
loadStdSkimPi0()
loadStdSkimPhoton()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()
loadStdDiLeptons(True)

cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4')


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.
    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists)
    summaryOfLists(lists)


# ISR cc skim
from skim.quarkonium import ISRpipiccList
add_skim('ISRpipicc', ISRpipiccList())

# BtoPi0Pi0 Skim
from skim.btocharmless import BtoPi0Pi0List
add_skim('BtoPi0Pi0', BtoPi0Pi0List())

# Tau Skim
from skim.taupair import TauLFVList
add_skim('TauLFV', TauLFVList())


# TCPV Skim
from skim.tcpv import TCPVList
add_skim('TCPV', TCPVList())


setSkimLogging()
process(analysis_main)

# print out the summary
print(statistics)
