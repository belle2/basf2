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

set_log_level(LogLevel.INFO)

fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]
inputMdstList(filelist)

loadStdCharged()
loadStdPi0()
loadStdKS()
loadStdLightMesons()
loadStdPhoton()
loadStdPhotonE15()

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.
    """
    skimOutputUdst(label, lists)
    summaryOfLists(lists)


# Double Charm Skim
from DoubleCharm_List import *
add_skim('DC', DoubleCharmList())

# Tau Skim
from Tau_List import *
add_skim('TauLFV', TauLFVList())

# EWP Skim
from EWP_List import *
add_skim('BtoXgamma', B2XgammaList())

add_skim('BtoXll', B2XllList())

# Had Skim
from CharmlessHad_List import *
add_skim('CharmlessHad', CharmlessHadList())

# TCPV Skim
from TCPV_List import *
add_skim('TCPV', TCPVList())

# SL Skim
from Semileptonic_List import *
add_skim('SLUntagged', SemileptonicList())

add_skim('LeptonicUntagged', LeptonicList())

# Charm Skim
from Charm_List import *
add_skim('CharmRare', CharmRareList())

# FEI Skim

process(analysis_main)

# print out the summary
print(statistics)
