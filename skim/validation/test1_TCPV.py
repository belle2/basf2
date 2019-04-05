#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../TDCP.dst.root</input>
  <output>../TDCP.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""


import basf2
import modularAnalysis as ma
from skimExpertFunctions import *

from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import loadStdLightMesons
from skim.standardlists.dileptons import loadStdDiLeptons

tcpvskimpath = Path()

fileList = ['TCPV.dst.root']

inputMdstList('default', fileList, path=tcpvskimpath)


loadStdSkimPi0(path=tcpvskimpath)
loadStdSkimPhoton(path=tcpvskimpath)
stdPi0s('loose', path=tcpvskimpath)
stdPi('loose', path=tcpvskimpath)
stdK('loose', path=tcpvskimpath)
stdE('loose', path=tcpvskimpath)
stdMu('loose', path=tcpvskimpath)
stdPi('all', path=tcpvskimpath)
stdPhotons('loose', path=tcpvskimpath)
stdKshorts(path=tcpvskimpath)
loadStdDiLeptons(True, path=tcpvskimpath)
loadStdLightMesons(path=tcpvskimpath)
cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=tcpvskimpath)


# TCPV  B0 skim
from skim.tcpv import TCPVList
TCPVList = TCPVList(path=tcpvskimpath)
ma.skimOutputUdst('TCPV.udst.root', TCPVList, path=tcpvskimpath)
ma.summaryOfLists(TCPVList, path=tcpvskimpath)

setSkimLogging(path=tcpvskimpath)
process(tcpvskimpath)

# print out the summary
print(statistics)
