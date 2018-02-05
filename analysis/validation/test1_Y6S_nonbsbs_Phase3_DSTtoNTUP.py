#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../Y6S_nonbsbs_Phase3.dst.root</input>
  <output>../Y6S_nonbsbs_Phase3.ntup.root</output>
  <contact>Cate MacQueen, cmq.centaurus@gmail.com</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdCharged import *
from stdV0s import *
from stdPhotons import *
from stdPi0s import *

from beamparameters import add_beamparameters
# create main path
main = create_path()
# add beam parameters
beamparameters = add_beamparameters(main, "Y6S")

inputMdst('default', '../Y6S_nonbsbs_Phase3.dst.root')
loadStdCharged()

# ----> NtupleMaker module
ntupleFile('../Y6S_nonbsbs_Phase3.ntup.root')

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
