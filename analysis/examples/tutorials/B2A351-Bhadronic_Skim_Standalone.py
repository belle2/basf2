#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################
#
# Stuck? Ask for help at questions.belle2.org
#
# Hadronic skim with full reconstruction
# R. Cheaib, May 30,2017
#
###############################

from basf2 import *
from modularAnalysis import *  # Standard Analysis Tools
from stdCharged import *  # Standard Charged Particle lists

fileList =\
    ['/group/belle2/tutorial/release_01-00-00/' +
     'mdst_001724_prod00000962_task00001729.root'
     ]
inputMdstList('default', fileList)

# Loading Charged particle lists: kaons, pions, leptons
stdPi('loose')
stdK('loose')
stdE('loose')
stdMu('loose')
stdPi('all')
stdK('all')
stdE('all')
stdMu('all')

# Hadronic Skim with Full  Reconstruction

from BHadronic_List import *
HadronicList = BHadronic()
skimOutputUdst('BHadronic_', HadronicList)  # output to Udst file
summaryOfLists(HadronicList)  # Print out Particle List statistics

process(analysis_main)

# print out the summary
print(statistics)
