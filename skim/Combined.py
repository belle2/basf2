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

loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()
loadStdDiLeptons(True)


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
from BtoXgamma_List import *
add_skim('12160100', B2XgammaList())
from BtoXll_List import *
add_skim('12160200', B2XllList())

# Had Skim
from CharmlessHad_List import *
add_skim('CharmlessHad', CharmlessHadList())

# TCPV Skim
from TCPV_List import *
add_skim('TCPV', TCPVList())

# SL Skim
from SLUntagged_List import *
add_skim('SLUntagged', SemileptonicList())
from LeptonicUntagged_List import *
add_skim('LeptonicUntagged', LeptonicList())

# Charm Skim
from CharmRare_List import *
add_skim('CharmRare', CharmRareList())

# FEI Skim

process(analysis_main)

# print out the summary
print(statistics)
