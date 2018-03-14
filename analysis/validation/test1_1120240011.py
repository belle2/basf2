#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1120240011.dst.root</input>
  <output>../1120240011.ntup.root</output>
  <contact>Sam Cunliffe; sam.cunliffe@desy.de</contact>
</header>
"""
# Validation script to reconstruct the decay:
#    B0 -> K*0 mu+ mu-
#           |
#           +-> K+ pi-
#
# Contributors: Sam Cunliffe (October 2017)

from basf2 import process, statistics
from vertex import *
from modularAnalysis import analysis_main, inputMdst, fillParticleList, \
    reconstructDecay, matchMCTruth, applyCuts, ntupleFile, ntupleTree
from stdCharged import stdPi, stdK, stdMu

# load input ROOT file
inputMdst('default', '../1120240011.dst.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

stdPi('95eff')
stdK('95eff')
stdMu('95eff')

# reconstruct K*0:Kpi
reconstructDecay('K*0:Kpi -> K+:95eff pi-:95eff', '0.795 < M < 0.995')
matchMCTruth('K*0:Kpi')

# reconstruct B0:sig
reconstructDecay('B0:sig -> K*0:Kpi mu+:95eff mu-:95eff', 'Mbc > 5.2 and abs(deltaE) < 0.2')
matchMCTruth('B0:sig')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

only_parent_B0 = '^B0:sig -> [K*0 -> K+ pi-] mu+ mu-'
charged_tracks = 'B0:sig -> [K*0 -> ^K+ ^pi-] ^mu+ ^mu-'
full_decay_chain = '^B0:sig -> [^K*0 -> ^K+ ^pi-] ^mu+ ^mu-'
only_composites = '^B0:sig -> [^K*0 -> K+ pi-] mu+ mu-'

# minimal information to be saved to file: only write that which needs to be plotted
tools = ['EventMetaData', '^B0:sig']
tools += ['Kinematics', full_decay_chain]
tools += ['DeltaEMbc', only_parent_B0]
tools += ['InvMass', only_composites]
tools += ['CustomFloats[isSignal]', only_parent_B0]

# write out the flat ntuple
ntupleFile('../1120240011.ntup.root')
ntupleTree('h1', 'B0:sig', tools)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
