
"""
<header>
  <input>../TDCP.dst.root</input>
  <output>../TDCP.udst.root</output>
  <contact>reem.rasheed@iphc.cnrs.fr</contact>
</header>
"""

__author__ = "Reem Rasheed"


import sys
import os.path

from basf2 import *
from modularAnalysis import *
from analysisPath import analysis_main
from beamparameters import add_beamparameters
from skimExpertFunctions import *

tcpvskimpath = Path()
fileList = ['../TCPV.dst.root']
inputMdstList('MC9', fileList, path=tcpvskimpath)


# Hadronic B0 skim
from skim.tcpv import *
TCPVList = TCPVList(path=tcpvskimpath)
skimOutputUdst('../TCPV.udst.root', TCPVList, path=tcpvskimpath)
summaryOfLists(TCPVList, path=tcpvskimpath)


process(tcpvskimpath)

# print out the summary
print(statistics)
