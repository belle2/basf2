#!/usr/bin/evn python3
# -*- coding: utf-8 -*-

##################################################
#
# Charm SL skims
# J. Bennett, 5/Oct/2016
#
##################################################

from basf2 import *
from modularAnalysis import *
from stdFSParticles import *


fileList = [
    '/ghi/fs01/belle2/bdata/MC/release-00-08-00/DB00000208/MC8/prod00000962/s00/e0000/4S/r00000/mixed/sub00/' +
    'mdst_001724_prod00000962_task00001729.root']

inputMdstList('default', fileList)


stdPi()
stdK()
stdPi0s('veryLooseFit')

reconstructDecay('K_S0:all -> pi-:95eff pi+:95eff', '0.4 < M < 0.6', 1, True, analysis_main)
vertexKFit('K_S0:all', 0.0)
applyCuts('K_S0:all', '0.477614 < M < 0.517614')

fillParticleList('e+:std', 'electronID > 0.1 and chiProb > 0.001 and p > 0.25', True, analysis_main)
fillParticleList('mu+:std', 'muonID > 0.1 and chiProb > 0.001 and p > 0.25', True, analysis_main)

# CSL Skim
from CharmSemileptonic_List import *
CSLList = CharmSemileptonicList()
skimOutputUdst('CharmSemileptonic', CSLList)
summaryOfLists(CSLList)

process(analysis_main)

# print out the summary
print(statistics)
