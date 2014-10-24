#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <input>Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root</input>
  <output>Bd_JpsiKS_mumu.ntup.root</output>
  <contact>Luis Pesantez; pesantez@uni-bonn.de</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from stdLooseFSParticles import *

inputMdst('../Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()
stdFSParticles()
stdLooseFSParticles()

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
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
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
    'B-',
    'RecoStats',
    'B-',
    'DetectorStatsRec',
    'B-',
    'DetectorStatsSim',
    'B-']

ntupleTree('eventtuple', '', eventtools)

summaryOfLists(['J/psi', 'K_S0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print statistics
