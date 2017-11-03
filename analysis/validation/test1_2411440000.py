#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../2411440000.dst.root</input>
  <output>../2411440000.ntup.root</output>
  <contact>Yubo Li; liyb@pku.edu.cn</contact>
</header>
"""

#######################################################
#
#
#         e+ e- -> gamma_{ISR} pi+ pi- Jpsi
#                                       |
#                                       +-> e+ e-
#
#
#######################################################

from basf2 import *
from modularAnalysis import inputMdst
from modularAnalysis import reconstructDecay
from modularAnalysis import fillParticleList
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import buildRestOfEvent
from modularAnalysis import ntupleFile
from modularAnalysis import ntupleTree
from variables import variables
from modularAnalysis import *

# Add signal MC files
inputFile = "../2411440000.dst.root"
inputMdstList('default', inputFile)

# fill candidates
fillParticleList('pi+:myPion', 'abs(dz) < 4.0 and abs(dr) < 0.5 and pionID > 0.6')
fillParticleList('pi-:myPion', 'abs(dz) < 4.0 and abs(dr) < 0.5 and pionID > 0.6')
fillParticleList('gamma:myISRGamma', 'E > 0.1')
fillParticleList('mu+:myMuon', 'abs(dz) < 4.0 and abs(dr) < 0.5')
fillParticleList('mu-:myMuon', 'abs(dz) < 4.0 and abs(dr) < 0.5')

# fill the largest energy gamma
rankByHighest('gamma:myISRGamma', 'E', 1)

reconstructDecay('J/psi:mumu -> mu+:myMuon mu-:myMuon', '')
massKFit('J/psi:mumu', 0.0)

reconstructDecay('vpho:myCombinations -> J/psi:mumu pi+:myPion pi-:myPion gamma:myISRGamma', '')
matchMCTruth('vpho:myCombinations')

# build the Rest of the track
buildRestOfEvent('vpho:myCombinations')

# create and fill flat Ntuple with MCTruth and kinematic information
tools = ['EventMetaData', '^vpho']
tools += ['DeltaEMbc', '^vpho']
tools += ['MCTruth', '^vpho']
tools += ['CustomFloats[Ecms:Eher:Eler:XAngle]', '^vpho']
tools += ['CustomFloats[electronID:muonID]', 'vpho -> [J/psi -> ^mu+ ^mu-] ^pi+ ^pi- gamma']
tools += ['CustomFloats[daughterInvariantMass(0,1)]', 'vpho -> [^J/psi -> mu+ mu-] pi+ pi- gamma']
tools += ['CustomFloats[m2Recoil]', '^vpho']
tools += ['CustomFloats[daughterInvariantMass(0,1,2)]', '^vpho']
tools += ['CustomFloats[useCMSFrame(cosTheta)]', '^vpho']
tools += ['CustomFloats[E]', 'vpho -> J/psi pi+ pi- ^gamma']

# write out the flat ntuple
ntupleFile('../2411440000.ntup.root')
ntupleTree('h1', 'vpho:myCombinations', tools)

# log info
set_log_level(LogLevel.INFO)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
