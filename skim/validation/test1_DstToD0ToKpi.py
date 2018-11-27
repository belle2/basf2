#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# Charm skims
# L.K. Li, 26/Nov/2018
#
######################################################

from ROOT import Belle2
from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import *

from stdCharged import *
from stdPi0s import *

set_log_level(LogLevel.INFO)
import sys
import os
import glob

fileList = [
    './WG6_DstToD0pi_D0ToKpi.dst.root'
]

# create path
myAna_Main = create_path()

inputMdstList('default', fileList, path=myAna_Main)

loadStdCharged(path=myAna_Main)

reconstructDecay(decayString='D0:kpi -> K-:loose pi+:loose',
                 cut='1.79 < M < 1.93 and useCMSFrame(p)>2.2', dmID=1, path=myAna_Main)
vertexKFit(list_name='D0:kpi', conf_level=0.001, path=myAna_Main)

DstList = []
reconstructDecay(decayString='D*+:sig -> D0:kpi pi+:all',
                 cut='0 < Q < 0.018', path=myAna_Main)
# vertexKFit( list_name='D*+:sig', conf_level=0.001,
#            constraint='ipprofile', path=myAna_Main )
vertexKFit(list_name='D*+:sig', conf_level=0.001, path=myAna_Main)

DstList.append('D*+:sig')

skimOutputUdst('WG6_DstToD0ToKpi', DstList, path=myAna_Main)

summaryOfLists(DstList)


setSkimLogging()
process(myAna_Main)

print(statistics)
