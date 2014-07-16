#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from stdLooseFSParticles import *

inputMdst('../Bu_D0pi,Kpi_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()
stdFSParticles()
stdLooseFSParticles()

makeParticle('D0 -> K-:all pi+:all', '1.7 < M <2.0')
applyCuts('D0', '1.81 < M < 1.91')
matchMCTruth('D0')

# Prepare the B candidates
makeParticle('B- -> D0 pi-:all', '5.2 < M < 5.4')
applyCuts('B-', '5.2 < M < 5.4')
matchMCTruth('B-')

# Prepare the ntuples
ntupleFile('../Bu_Dpi,Kpi.ntup.root')
tools = [
    'EventMetaData',
    'B-',
    'RecoStats',
    'B-',
    'DeltaEMbc',
    '^B- -> [anti-D0 -> K+ pi-] pi-',
    'MCTruth',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    'PID',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    'Track',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    'MCHierarchy',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    'MCKinematics',
    'B- -> [anti-D0 -> ^K+ ^pi-] ^pi-',
    'Kinematics',
    '^B- -> [^anti-D0 -> ^K+ ^pi-] ^pi-']

ntupleTree('Bu_Dpi_tuple', 'B-', tools)

##########
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

summaryOfLists(['D0', 'B-'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print statistics
