#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>./WG6_DstToD0pi_D0ToKpi.dst.root</input>
  <output>./WG6_DstToD0ToKpi.udst.root</output>
  <contact>lilongke@ihep.ac.cn</contact>
</header>
"""

import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert

from stdCharged import *
from stdPi0s import *

b2.set_log_level(b2.LogLevel.INFO)

fileList = [
    './WG6_DstToD0pi_D0ToKpi.dst.root'
]

# create path
path = b2.create_path()

ma.inputMdstList('MC9', fileList, path=path)

stdPi('loose', path=path)
stdK('loose', path=path)
stdPi('all', path=path)
stdK('all', path=path)
stdE('all', path=path)


from skim.charm import DstToD0PiD0ToHpJm
DstToD0PiD0ToHpJmList = DstToD0PiD0ToHpJm(path)

expert.skimOutputUdst('../WG6_DstToD0ToKpi.udst.root', DstToD0PiD0ToHpJmList, path=path)
ma.summaryOfLists(DstToD0PiD0ToHpJmList, path=path)


expert.setSkimLogging(path)
b2.process(path)

print(b2.statistics)
