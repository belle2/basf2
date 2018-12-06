#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../BBgeneric_Phase2.dst.root</input>
  <output>../BBgeneric_Phase2.ntup.root</output>
  <contact>Cate MacQueen; cmq.centaurus@gmail.com, Bryan Fulsom bryan.fulsom@pnnl.gov</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdCharged import stdPi

from beamparameters import add_beamparameters
# create main path
main = create_path()

inputMdst('default', '../BBgeneric_Phase2.dst.root')
stdPi('all')

# ----> NtupleMaker module
ntupleFile('../BBgeneric_Phase2.ntup.root')

# Save the tracks to ntuple
recoToolsPi = [
    'EventMetaData',
    '^pi+',
    'Kinematics',
    '^pi+',
    'InvMass',
    '^pi+',
    'Track',
    '^pi+',
    'MCTruth',
    '^pi+',
    'MCKinematics',
    '^pi+',
    'MCReconstructible',
    '^pi+',
    'PID',
    '^pi+'
]

# Save the truth tracks to ntuple
fillParticleListFromMC('pi+:gen', '')
truthToolsPi = [
    'EventMetaData',
    '^pi+',
    'Kinematics',
    '^pi+',
    'InvMass',
    '^pi+',
    'MCTruth',
    '^pi+',
    'MCKinematics',
    '^pi+',
    'MCReconstructible',
    '^pi+',
    'MCHierarchy',
    '^pi+'
]

# Save the photons to ntuple
fillParticleList('gamma:all', '')
recoToolsGamma = [
    'EventMetaData',
    '^gamma',
    'Kinematics',
    '^gamma',
    'MCTruth',
    '^gamma',
    'MCKinematics',
    '^gamma',
    'MCReconstructible',
    '^gamma',
    'Cluster',
    '^gamma'
]

# Save the truth photons to ntuple
fillParticleListFromMC('gamma:gen', '')
truthToolsGamma = [
    'EventMetaData',
    '^gamma',
    'Kinematics',
    '^gamma',
    'MCTruth',
    '^gamma',
    'MCKinematics',
    '^gamma',
    'MCReconstructible',
    '^gamma',
    'MCHierarchy',
    '^gamma'
]

# dump all event summary information
eventtools = [
    'EventMetaData',
    '^B-',
    'RecoStats',
    '^B-',
]

ntupleTree('pituple', 'pi+:all', recoToolsPi)
ntupleTree('truthpituple', 'pi+:gen', truthToolsPi)
ntupleTree('gammatuple', 'gamma:all', recoToolsGamma)
ntupleTree('truthgammatuple', 'gamma:gen', truthToolsGamma)
ntupleTree('eventtuple', '', eventtools)

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
