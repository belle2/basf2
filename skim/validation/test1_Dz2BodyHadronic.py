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
path = create_path()

inputMdstList('MC9', fileList, path=path)

stdPi('loose', path=path)
stdK('loose', path=path)
stdPi('all', path=path)
stdK('all', path=path)
stdE('all', path=path)


from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(path)

skimOutputUdst('../WG6_DstToD0ToKpi.udst.root', DstToD0PiD0ToHpJmList, path=path)
summaryOfLists(DstToD0PiD0ToHpJmList, path=path)


setSkimLogging(path)
process(path)

print(statistics)
