#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../TDCP.dst.root</input>
  <output>../TDCP.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""


import basf2 as b2
import modularAnalysis as ma
import skimExpertFunctions as expert

from stdCharged import stdE, stdK, stdMu, stdPi
from stdPhotons import stdPhotons, loadStdSkimPhoton
from stdPi0s import stdPi0s, loadStdSkimPi0
from stdV0s import stdKshorts
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
