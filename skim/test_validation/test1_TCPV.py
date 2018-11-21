
"""
<header>
  <input>../TDCP.dst.root</input>
  <output>../TDCP.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""

__author__ = "Reem Rasheed"


import sys

from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters
from skimExpertFunctions import *


from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *
from stdPi0s import *
from stdV0s import *
from stdLightMesons import *
from stdDiLeptons import *
from skimExpertFunctions import *


tcpvskimpath = Path()
fileList = ['../TCPV.dst.root']
inputMdstList('MC9', fileList, path=tcpvskimpath)


loadStdSkimPi0(path=tcpvskimpath)
loadStdSkimPhoton(path=tcpvskimpath)
stdPi0s('loose', path=tcpvskimpath)
loadStdCharged(path=tcpvskimpath)
stdPhotons('loose', path=tcpvskimpath)
loadStdKS(path=tcpvskimpath)
loadStdDiLeptons(True, path=tcpvskimpath)
loadStdLightMesons(path=tcpvskimpath)
cutAndCopyList('gamma:E15', 'gamma:loose', '1.4<E<4', path=tcpvskimpath)


# TCPV  B0 skim
from skim.tcpv import TCPVList
TCPVList = TCPVList(path=tcpvskimpath)
skimOutputUdst('../TCPV.udst.root', TCPVList, path=tcpvskimpath)
summaryOfLists(TCPVList, path=tcpvskimpath)


process(tcpvskimpath)

# print out the summary
print(statistics)
