#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../TDCP.dst.root</input>
  <output>../TDCP.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""


import modularAnalysis as ma
import skimExpertFunctions as expert

from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from skim.standardlists.lightmesons import loadStdLightMesons
from skim.standardlists.dileptons import loadStdDiLeptons

tcpvskimpath = b2.Path()

fileList = ['../TCPV.dst.root']

ma.inputMdstList('default', fileList, path=tcpvskimpath)


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
ma.cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=tcpvskimpath)


# TCPV  B0 skim
from skim.tcpv import TCPVList
TCPVList = TCPVList(path=tcpvskimpath)
ma.expert.skimOutputUdst('../TCPV.udst.root', TCPVList, path=tcpvskimpath)
ma.summaryOfLists(TCPVList, path=tcpvskimpath)

expert.setSkimLogging(path=tcpvskimpath)
b2.process(tcpvskimpath)

# print out the summary
print(b2.statistics)
