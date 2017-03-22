#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>Bd_rho0gamma_GENSIMRECtoDST.dst.root</input>
  <output>Bd_rho0gamma.ntup.root</output>
  <contact>Jorge Martinez-Ortega; jmartinez@fis.cinvestav.mx</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdPhotons import *

inputMdst('default', '../Bd_rho0gamma_GENSIMRECtoDST.dst.root')
loadStdCharged()
loadStdPhoton()

reconstructDecay('rho0 -> pi-:all pi+:all', '0.6 < M < 1.2')
matchMCTruth('rho0')

# Prepare the B candidates
reconstructDecay('B0 -> rho0 gamma:all', '5.2 < M < 5.4')
matchMCTruth('B0')

ntupleFile('../Bd_rho0gamma.ntup.root')
tools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'Kinematics',
    '^B0 -> [^rho0 -> ^pi+ ^pi-] ^gamma',
    'MCTruth',
    '^B0 -> [^rho0 -> ^pi+ ^pi-] ^gamma',
    'DeltaEMbc',
    '^B0 -> [rho0 -> pi+ pi-] gamma',
    'MCHierarchy',
    'B0 -> [rho0 -> ^pi+ ^pi-] ^gamma',
    'PID',
    'B0 -> [rho0 -> ^pi+ ^pi-] gamma',
]

ntupleTree('Bd_rho0gamma_tuple', 'B0', tools)

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

summaryOfLists(['rho0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
