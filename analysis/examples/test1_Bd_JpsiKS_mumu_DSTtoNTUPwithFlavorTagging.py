#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <input>Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root</input>
  <output>Bd_JpsiKS_mumu.ntup.root</output>
  <contact>Fernando Abudinen; abudinen@mpp.mpg.de</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from FlavorTagger import *
from stdFSParticles import *
from stdLooseFSParticles import *

inputMdst('../Bd_JpsiKS_mumu_GENSIMRECtoDST.dst.root')
stdFSParticles()
stdLooseFSParticles()

# Signal side B_sig
fillParticleList('mu+', 'muid >= 0.1')
fillParticleList('pi+', 'piid >= 0.1')

reconstructDecay('J/psi -> mu+ mu-', '3.0<=M<=3.2', 1)
reconstructDecay('K_S0 -> pi+ pi-', '0.25<=M<=0.75', 1)
reconstructDecay('B0 -> K_S0 J/psi', '5.2 <= M <= 5.4', 1)

# Use only correctly reconstructed B_sig
matchMCTruth('B0')
applyCuts('B0', 'isSignal > 0.5')

# Tag side B_tag
buildRestOfEvent('B0')
buildContinuumSuppression('B0')

# Flavor Tagging
FlavorTagger(weightFiles='B2JpsiKs_mu')

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
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    'FlavorTagging',
    '^B0 -> [J/psi -> mu+ mu-] [K_S0 -> pi+ pi-]',
    ]

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
    'B-',
    ]

ntupleTree('eventtuple', '', eventtools)

summaryOfLists(['J/psi', 'K_S0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print statistics
