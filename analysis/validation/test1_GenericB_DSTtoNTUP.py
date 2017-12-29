#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>GenericB_GENSIMRECtoDST.dst.root</input>
  <output>GenericB.ntup.root</output>
  <contact>Jorge Martinez-Ortega; jmartinez@fis.cinvestav.mx</contact>
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
from stdLightMesons import *

inputMdst('default', '../GenericB_GENSIMRECtoDST.dst.root')
loadStdCharged()
stdPhotons('loose')  # loose list also creates 'all' list
loadStdPi0('all')
loadStdLoosePi0()
loadStdKS()
loadStdLightMesons()


# ----> NtupleMaker module
ntupleFile('../GenericB.ntup.root')

##########
# Save the tracks to ntuple
recoToolsPi = [
    'EventMetaData',
    '^pi+',
    'Kinematics',
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
    '^pi+',
]
ntupleTree('pituple', 'pi+:all', recoToolsPi)
# Save the truth tracks to ntuple
fillParticleListFromMC('pi+:gen', '')
truthToolsPi = [
    'EventMetaData',
    '^pi+',
    'Kinematics',
    '^pi+',
    'MCTruth',
    '^pi+',
    'MCKinematics',
    '^pi+',
    'MCReconstructible',
    '^pi+',
    'MCHierarchy',
    '^pi+',
]
ntupleTree('truthpituple', 'pi+:gen', truthToolsPi)

###########
# Save the photons to ntuple
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
    '^gamma',
]
ntupleTree('gammatuple', 'gamma:all', recoToolsGamma)
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
    '^gamma',
]
ntupleTree('truthgammatuple', 'gamma:gen', truthToolsGamma)

###########
# check the pi0 list for resolution etc.
matchMCTruth('pi0:all')
recoToolsPi0 = [
    'EventMetaData',
    '^pi0',
    'MCTruth',
    '^pi0 -> ^gamma ^gamma',
    'MCKinematics',
    '^pi0 -> ^gamma ^gamma',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    'InvMass',
    '^pi0',
    'MCHierarchy',
    '^pi0 -> ^gamma ^gamma',
]
ntupleTree('pi0tuple', 'pi0:all', recoToolsPi0)
# Save the truth pi0s to ntuple
reconstructDecay('pi0:gen -> gamma:gen gamma:gen', '0.1 < M < 0.15')
matchMCTruth('pi0:gen')
applyCuts('pi0:gen', 'isSignal > 0.5')
truthToolsPi0 = [
    'EventMetaData',
    '^pi0',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    'InvMass',
    '^pi0',
]
ntupleTree('truthpi0tuple', 'pi0:gen', truthToolsPi0)

##########
# Check the KS0 candidates
matchMCTruth('K_S0:all')
recoToolsKs = [
    'EventMetaData',
    '^K_S0',
    'MCTruth',
    '^K_S0 -> ^pi- ^pi+',
    'Kinematics',
    '^K_S0 -> ^pi- ^pi+',
    'InvMass',
    '^K_S0',
    'Vertex',
    'K_S0 -> ^pi- ^pi+',
    'MCVertex',
    'K_S0 -> ^pi- ^pi+',
]
ntupleTree('kstuple', 'K_S0:all', recoToolsKs)

##########
# dump all event summary information
eventtools = [
    'EventMetaData',
    '^B-',
    'RecoStats',
    '^B-',
]
ntupleTree('eventtuple', '', eventtools)

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
