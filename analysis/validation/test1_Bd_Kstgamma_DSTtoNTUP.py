#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from stdLooseFSParticles import *
from stdLightMesons import *

inputMdst('../Bd_Kstgamma_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()
stdFSParticles()
stdLooseFSParticles()
stdLightMesons()

makeParticle('K*0', 313, ['StdVeryLooseK-', 'StdVeryLoosePi+'], 0.6, 1.2)
matchMCTruth('K*0')

# Prepare the B candidates
makeParticle('B0toK*0gamma', 511, ['K*0', 'StdPhoton'], 5.2, 5.4)
matchMCTruth('B0toK*0gamma')

ntupleFile('../Bd_Kstgamma.ntup.root')
tools = [
    'EventMetaData',
    'B0',
    'RecoStats',
    'B0',
    'Kinematics',
    '^B0 -> [^K*0 -> ^K+ ^pi-] ^gamma',
    'MCTruth',
    'B0 -> [K*0 -> ^K+ ^pi-] ^gamma',
    'DeltaEMbc',
    '^B0 -> [K*0 -> K+ pi-] gamma',
    'MCHierarchy',
    'B0 -> [K*0 -> ^K+ ^pi-] ^gamma',
    'PID',
    'B0 -> [K*0 -> ^K+ ^pi-] gamma',
    ]
ntupleTree('Bd_Kstgamma_tuple', 'B0toK*0gamma', tools)

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

summaryOfLists(['K*0', 'B0toK*0gamma'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print statistics
