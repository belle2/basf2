#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root</input>
  <output>Bd_JpsiKS_mumu.ntup.root</output>
  <contact>Jorge Martinez-Ortega; jmartinez@fis.cinvestav.mx</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdCharged import *

inputMdst('default', '../Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root')
loadStdCharged()
# fillParticleList('mu-:all','')
# fillParticleList('pi-:all','')


reconstructDecay('J/psi -> mu-:all mu+:all', '2.8 < M < 3.3')
matchMCTruth('J/psi')
reconstructDecay('K_S0 -> pi-:all pi+:all', '0.4 < M < 0.6')
matchMCTruth('K_S0')

# Prepare the B candidates
reconstructDecay('B0 -> J/psi K_S0', '5.2 < M < 5.4')
matchMCTruth('B0')

# ----> NtupleMaker module
ntupleFile('../Bd_JpsiKS_mumu.ntup.root')
tools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'Kinematics',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'MCTruth',
    '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]',
    'DeltaEMbc',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    'MCHierarchy',
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    'PID',
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']

ntupleTree('Bd_JpsiKS_tuple', 'B0', tools)

# dump all event summary information
eventtools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'DetectorStatsRec',
    'B0',
    'DetectorStatsSim',
    'B0']

ntupleTree('eventtuple', '', eventtools)

summaryOfLists(['J/psi', 'K_S0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
