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

makeParticle('D0', 421, ['StdVeryLooseK-', 'StdVeryLoosePi+'], 1.7, 2.0)
applyCuts('D0', ['M 1.81:1.91'])
matchMCTruth('D0')

# Prepare the B candidates
makeParticle('B-toD0pi', -521, ['D0', 'StdVeryLoosePi-'], 5.2, 5.4)
applyCuts('B-toD0pi', ['M 5.2:5.4'])
matchMCTruth('B-toD0pi')

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
    '^B- -> [^anti-D0 -> ^K+ ^pi-] ^pi-',
    ]
ntupleTree('Bu_Dpi_tuple', 'B-toD0pi', tools)

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
    'B-',
    ]
ntupleTree('eventtuple', '', eventtools)

summaryOfLists(['D0', 'B-toD0pi'])

# ----> start processing of modules
process(main)

# ----> Print call statistics
print statistics
