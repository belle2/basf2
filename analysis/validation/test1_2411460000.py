#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../2411460000.dst.root</input>
  <output>../2411460000.ntup.root</output>
  <contact>Sen Jia; jiasen@buaa.edu.cn</contact>
</header>
"""

#######################################################
#                                                     #
#     e+ e- -> gamma_{ISR} pi+ pi- psi(2S)            #
#                                     |               #
#                           pi+ pi- J/psi(->mu+mu-)   #
#                                                     #
#######################################################

from basf2 import *
from stdCharged import *
from variables import *
from stdPhotons import *
from modularAnalysis import *

inputFile = "../2411460000.dst.root"
inputMdstList('default', inputFile)

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

stdMu('95eff')
stdPi('95eff')
stdPhotons('tight')

# fill the largest energy gamma
rankByHighest('gamma:tight', 'E', 1)

reconstructDecay('J/psi:mumu -> mu+:95eff mu-:95eff', '')
massKFit('J/psi:mumu', 0.0)

reconstructDecay('psi(2S):mypsi -> J/psi:mumu pi+:95eff pi-:95eff', '')
matchMCTruth('psi(2S):mypsi')

reconstructDecay('vpho:myCombinations -> psi(2S):mypsi pi+:95eff pi-:95eff gamma:tight', '')
matchMCTruth('vpho:myCombinations')

# build the Rest of the track
buildRestOfEvent('vpho:myCombinations')

# create and fill flat Ntuple with MCTruth and kinematic information
tools = ['EventMetaData', '^vpho']
tools += ['DeltaEMbc', '^vpho']
tools += ['MCTruth', '^vpho']
tools += ['CustomFloats[Ecms:Eher:Eler:XAngle]', '^vpho']
tools += ['CustomFloats[electronID:muonID:pionID]', 'vpho -> [psi(2S) -> [J/psi -> ^mu+ ^mu-] ^pi+ ^pi-] ^pi+ ^pi- gamma']
tools += ['CustomFloats[daughterInvariantMass(0,1)]', 'vpho -> [psi(2S) -> [^J/psi -> mu+ mu-] pi+ pi-] pi+ pi- gamma']
tools += ['CustomFloats[daughterInvariantMass(0,1,2)]', 'vpho -> ^psi(2S) pi+ pi- gamma ']
tools += ['CustomFloats[m2Recoil]', '^vpho']
tools += ['CustomFloats[useCMSFrame(cosTheta)]', '^vpho']
tools += ['CustomFloats[daughterInvariantMass(0,1,2)]', '^vpho']
tools += ['CustomFloats[daughterInvariantMass(1,2)]', 'vpho -> ^psi(2S) pi+ pi- gamma']
tools += ['CustomFloats[E]', 'vpho -> psi(2S) pi+ pi- ^gamma']

# write out the flat ntuple
ntupleFile('../2411460000.ntup.root')
ntupleTree('h1', 'vpho:myCombinations', tools)

# log info
set_log_level(LogLevel.INFO)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
