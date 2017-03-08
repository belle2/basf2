#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# EWP standalone skim steering
# P. Urquijo, F. Tenchini 6/Jan/2015
#
######################################################

from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPi0s import *
from stdPhotons import *
from stdV0s import *
from stdLightMesons import *

set_log_level(LogLevel.INFO)

filelist = \
    ['/ghi/fs01/belle2/bdata/MC/release-00-07-02/DBxxxxxxxx/MC7/prod00000787/s00/e0000/4S/r00000/mixed/sub00/' +
     'mdst_000001_prod00000787_task00000001.root'
     ]  # This is an MC7 file used for testing since we don't have MC8 yet; please modify accordingly
inputMdstList('default', filelist)

stdPi('95eff')
stdK('95eff')
# lepton efficiency benchmark is missing, will load mu+:all and e+:all instead
stdPhotons('loose')
stdPi0s('loose')
stdKshorts()
loadStdCharged()  # needed for light mesons; also loads mu+:all and e+:all
loadStdLightMesons()

# EWP Skim
from EWP_List import *
XgammaList = B2XgammaList()
skimOutputUdst('BtoXgamma_Standalone', XgammaList)
summaryOfLists(XgammaList)

XllList = B2XllList()
skimOutputUdst('BtoXll_Standalone', XllList)
summaryOfLists(XllList)

# printDataStore()

process(analysis_main)

# print out the summary
print(statistics)
