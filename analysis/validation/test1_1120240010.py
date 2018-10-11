#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1120240010.dst.root</input>
  <output>../1120240010.ntup.root</output>
  <contact>Sam Cunliffe; sam.cunliffe@desy.de</contact>
</header>
"""
# Validation script to reconstruct the decay:
#    B0 -> K*0 e+ e-
#           |
#           +-> K+ pi-
#
# Contributors: Sam Cunliffe (October 2017)

from basf2 import process, statistics
from modularAnalysis import analysis_main, inputMdst, fillParticleList, \
    reconstructDecay, matchMCTruth, applyCuts, ntupleFile, ntupleTree
from stdCharged import stdPi, stdK, stdE

# load input ROOT file
inputMdst('default', '../1120240010.dst.root')

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

stdPi('95eff')
stdK('95eff')
stdE('95eff')

# reconstruct K*0:Kpi
reconstructDecay('K*0:Kpi -> K+:95eff pi-:95eff', '0.795 < M < 0.995')
matchMCTruth('K*0:Kpi')

# reconstruct B0:sig
reconstructDecay('B0:sig -> K*0:Kpi e+:95eff e-:95eff', 'Mbc > 5.2 and abs(deltaE) < 0.2')
matchMCTruth('B0:sig')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

only_parent_B0 = '^B0:sig -> [K*0 -> K+ pi-] e+ e-'
charged_tracks = 'B0:sig -> [K*0 -> ^K+ ^pi-] ^e+ ^e-'
full_decay_chain = '^B0:sig -> [^K*0 -> ^K+ ^pi-] ^e+ ^e-'
only_composites = '^B0:sig -> [^K*0 -> K+ pi-] e+ e-'

# minimal information to be saved to file: only write that which needs to be plotted
tools = ['EventMetaData', '^B0:sig']
tools += ['Kinematics', full_decay_chain]
tools += ['DeltaEMbc', only_parent_B0]
tools += ['InvMass', only_composites]

# write out the flat ntuple
ntupleFile('../1120240010.ntup.root')
ntupleTree('h1', 'B0:sig', tools)

# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
