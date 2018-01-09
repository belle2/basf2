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
    ['/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
     'mdst_001724_prod00000962_task00001729.root'
     ]

inputMdstList('default', fileList)


loadStdCharged()  # Loading Charged particle lists: kaons, pions, leptons

# Hadronic Skim with Full  Reconstruction

from Breco_List import *
BList = Breco()
skimOutputUdst('Breco', BList)  # output to Udst file
summaryOfLists(BList)  # Print out Particle List statistics

process(analysis_main)

# print out the summary
print(statistics)
