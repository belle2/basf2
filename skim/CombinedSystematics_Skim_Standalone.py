#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Combined all systematic skims at once
#  * Final state particles
#  * Charged tracks
#  * Lambda
#  * Radiative Muon pair
#  * Resonance discovery
#
# R. Cheaib , May 28, 2018
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
gb2_setuprel = 'release-02-00-00'

from skimExpertFunctions import *
fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002288/e0000/4S/r00000/mixed/sub00/' +
    'mdst_000001_prod00002288_task00000001.root'
]

inputMdstList('MC9', fileList)
loadStdCharged()
stdPi0s('looseFit')
stdPhotons('loose')
setSkimLogging()


def add_skim(label, lists):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.
    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists)
    summaryOfLists(lists)


# Systematics skim
from Systematics_List import *
add_skim('Systematics', SystematicsList())

# Systematics Lambda Skim
from SystematicsLambda_List import *
add_skim('SystematicsLambda', SystematicsLambdaList())

# Systematics Tracking
from SystematicsTracking_List import *
add_skim('SystematicsTracking', SystematicsTrackingList())

# Resonan ce
from Resonance_List import *
add_skim('Resonance', ResonanceList())

# Systematics Rad mu mu
from SystematicsRadMuMu_List import *
add_skim('SystematicsRadMuMu', SystematicsRadMuMuList())

# Systematics Rad mu mu
from SystematicsRadEE_List import *
add_skim('SystematicsRadMuMu', SystematicsRadMuMuList())

process(analysis_main)

# print out the summary
print(statistics)
