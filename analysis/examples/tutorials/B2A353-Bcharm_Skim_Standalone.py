#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################
#
# Hadronic skim with full reconstruction
# R. Cheaib, May 30,2017
#
###############################

from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
from stdCharged import *  # Standard Charged Particle lists
from stdPi0s import *  # Standard Pi0 list
from stdCharm import *  # D0, D+, D*0 and D*+ list
from stdV0s import *  # Standard KS

fileList =\
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
     'mdst_001724_prod00000962_task00001729.root'
     ]


inputMdstList('default', fileList)


loadStdPi0()
loadStdKS()
loadStdCharged()  # Loading Charged particle lists: kaons, pions, leptons
# load D mesons from stdCharm.py
loadStdD0()
loadStdDplus()
loadStdDstar0()
loadStdDstarPlus()

from Bcharm_List import *
BList = Bcharm()
skimOutputUdst('Bcharm', BList)  # output to Udst file
summaryOfLists(BList)  # Print out Particle List statistics

process(analysis_main)

# print out the summary
print(statistics)
