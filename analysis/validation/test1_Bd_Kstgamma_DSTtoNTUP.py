#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>Bd_Kstgamma_GENSIMRECtoDST.dst.root</input>
  <output>Bd_Kstgamma.ntup.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

inputMdst('default', '../Bd_Kstgamma_GENSIMRECtoDST.dst.root')
loadStdCharged()
loadStdPhoton()

reconstructDecay('K*0 -> K+:all pi-:all', '0.6 < M < 1.2')
matchMCTruth('K*0')

# Prepare the B candidates
reconstructDecay('B0 -> K*0 gamma:all', '5.2 < M < 5.4')
matchMCTruth('B0')

ntupleFile('../Bd_Kstgamma.ntup.root')
tools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'Kinematics',
    '^B0 -> [^K*0 -> ^K+ ^pi-] ^gamma',
    'MCTruth',
    '^B0 -> [^K*0 -> ^K+ ^pi-] ^gamma',
    'DeltaEMbc',
    '^B0 -> [K*0 -> K+ pi-] gamma',
    'MCHierarchy',
    'B0 -> [K*0 -> ^K+ ^pi-] ^gamma',
    'PID',
    'B0 -> [K*0 -> ^K+ ^pi-] gamma',
]

ntupleTree('Bd_Kstgamma_tuple', 'B0', tools)

##########
# dump all event summary information
eventtools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'DetectorStatsRec',
    'B0',
    'DetectorStatsSim',
    'B0',
]

ntupleTree('eventtuple', '', eventtools)

summaryOfLists(['K*0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
