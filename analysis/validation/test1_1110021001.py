#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../1110021001.dst.root</input>
  <output>../1110021001.ntup.root</output>
  <contact>Saurabh Sandilya; saurabhsandilya@gmail.com</contact>
</header>
"""

#######################################################
#
# Obtain validation plots from the decay:
#
#
#    B0 -> K*0 + gamma
#           |
#           +-> K+ pi-
#
#
# Contributors: Saurabh Sandilya
#               Sam Cunliffe (Last update:Oct 31, 2017)
######################################################

from basf2 import process, statistics
from modularAnalysis import *
from stdFSParticles import *
from variables import variables

# load input ROOT file
inputMdst('default', '../1110021001.dst.root')
# default: for analysis of Belle II MC samples produced
# with releases with release-00-08-00 or newer

# --------------------------------------------------
# Create and fill final state ParticleLists
# --------------------------------------------------

stdK('95eff')
stdPi('95eff')
stdPhotons('loose')

# reconstruct K*0:kpi
reconstructDecay('K*0:kpi -> K+:95eff pi-:95eff', '0.6 < M < 1.2')
matchMCTruth('K*0:kpi')

# reconstruct B0:sig
reconstructDecay('B0:sig -> K*0:kpi gamma:loose', '4.8 < M < 5.8')
matchMCTruth('B0:sig')

# --------------------------------------------------
# write out useful information to a ROOT file
# --------------------------------------------------

# information to be saved to file
tools = ['EventMetaData', '^B0:sig']
tools += ['RecoStats', '^B0:sig']
tools += ['Kinematics', '^B0:sig -> [^K*0:kpi -> ^K+:95eff ^pi-:95eff] ^gamma:loose']
tools += ['InvMass', '^B0:sig -> [^K*0:kpi -> K+:95eff pi-:95eff] gamma:loose']
tools += ['DeltaEMbc', '^B0:sig -> [K*0:kpi -> K+:95eff pi-:95eff] gamma:loose']
variables.addAlias("Egamma", "useRestFrame(daughter(1,E))")
tools += ['CustomFloats[Egamma]', '^B0:sig -> [K*0:kpi -> K+:95eff pi-:95eff] gamma:loose']


# write out the flat ntuple
ntupleFile('../1110021001.ntup.root')
ntupleTree('h1', 'B0:sig', tools)


# --------------------------------------------------
# Process the events and print call statistics
# --------------------------------------------------

process(analysis_main)
print(statistics)
