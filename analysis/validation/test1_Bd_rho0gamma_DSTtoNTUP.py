#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>Bd_rho0gamma_GENSIMRECtoDST.dst.root</input>
  <output>Bd_rho0gamma.ntup.root</output>
  <contact>Saurabh Sandilya; saurabhsandilya@gmail.com</contact>
</header>
"""
#######################################################                         
#                                                                               
# Obtain Delta E and Mbc distribution from the decay:                           
#                                                                               
#                                                                               
#    B0 -> rho0 + gamma                                                          
#           |                                                                   
#           +-> pi+ pi-                                                          
#                                                                               
#                                                                               
# Contributors: Saurabh Sandilya (April 2017)                                   
#                                                                               
######################################################                          

from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *

gb2_setuprel="release-00-08-00"

inputMdst('default', '/gpfs/home/belle/saurabh/ewp2/validation/mdst-rfiles/mdst-1110021010-1.root')

fillParticleList('pi+:all', 'chiProb > 0.001 and abs(d0) < 2 and abs(z0) < 4 and piid > 0.1')

fillParticleList('gamma:all', 'E > 0.050 and clusterE9E25 > 0.8')

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
