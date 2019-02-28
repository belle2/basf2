#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>./WG6_DstToD0pi_D0ToKpi.dst.root</input>
  <output>./WG6_DstToD0ToKpi.udst.root</output>
  <contact>lilongke@ihep.ac.cn</contact>
</header>
"""

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

inputMdstList('MC9', fileList, path=myAna_Main)

stdPi('loose', path=myAna_Main)
stdK('loose', path=myAna_Main)
stdPi('all', path=myAna_Main)
stdK('all', path=myAna_Main)
stdE('all', path=myAna_Main)


from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(myAna_Main)

skimOutputUdst('WG6_DstToD0ToKpi', DstToD0PiD0ToHpJmList, path=myAna_Main)
summaryOfLists(DstList, path=myAna_Main)


setSkimLogging()
process(myAna_Main)

print(statistics)
