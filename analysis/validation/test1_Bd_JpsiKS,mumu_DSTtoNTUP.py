#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from stdLooseFSParticles import *

inputMdst('../Bd_JpsiKS,mumu_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()
stdFSParticles()
stdLooseFSParticles()

makeParticle('J/psi', 443, ['StdVeryLooseMu-', 'StdVeryLooseMu+'], 2.8, 3.3)
matchMCTruth('J/psi')
makeParticle('K_S0', 310, ['StdVeryLoosePi-', 'StdVeryLoosePi+'], 0.4, 0.6)
matchMCTruth('K_S0')

# Prepare the B candidates
makeParticle('B0toJpsiKS', 511, ['J/psi', 'K_S0'], 5.2, 5.4)
matchMCTruth('B0toJpsiKS')

# ----> NtupleMaker module
ntupleFile('../Bd_JpsiKS,mumu.ntup.root')
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
    'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]',
    ]
ntupleTree('Bd_JpsiKS_tuple', 'B0toJpsiKS', tools)

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

summaryOfLists(['J/psi', 'K_S0', 'B0toJpsiKS'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print statistics

