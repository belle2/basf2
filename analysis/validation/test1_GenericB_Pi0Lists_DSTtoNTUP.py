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
from stdPhotons import *
from stdPi0s import *


inputMdst('default', '../GenericB_GENSIMRECtoDST.dst.root')
# inputMdst('default', '/gpfs/fs02/belle2/users/merola/GenericB_GENSIMRECtoDST.dst.root')

stdPhotons('loose')
stdPhotons('tight')
stdPhotons('pi0')
stdPhotons('pi0highE')

stdPi0s('veryLoose')
stdPi0s('loose')


# ----> NtupleMaker module
ntupleFile('../GenericB.ntup.root')

# ----> if run on the grid
# ntupleFile('GenericB.ntup.root')


# check the pi0 list for resolution etc.

matchMCTruth('pi0:loose')
matchMCTruth('pi0:veryLoose')

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
    'CustomFloats[cosTheta]',
    '^pi0',
]


# Save the truth pi0s to ntuple
fillParticleListFromMC('gamma:gen', '')
reconstructDecay('pi0:gen -> gamma:gen gamma:gen', '0.09 < M < 0.165')
matchMCTruth('pi0:gen')
applyCuts('pi0:gen', 'isSignal > 0.5')
truthToolsPi0 = [
    'EventMetaData',
    '^pi0',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    'MCKinematics',
    '^pi0 -> ^gamma ^gamma',
    'InvMass',
    '^pi0',
    'MCTruth',
    '^pi0 -> ^gamma ^gamma',
    'CustomFloats[cosTheta]',
    '^pi0',
]


ntupleTree('pi0s_truth', 'pi0:gen', truthToolsPi0)
ntupleTree('pi0s_veryLoose', 'pi0:veryLoose', recoToolsPi0)
ntupleTree('pi0s_Loose', 'pi0:loose', recoToolsPi0)


# dump all event summary information
eventtools = [
    'EventMetaData',
    '^B-',
    'RecoStats',
    '^B-',
    'DetectorStatsRec',
    '^B-',
    'DetectorStatsSim',
    '^B-',
]
ntupleTree('eventtuple', '', eventtools)

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
