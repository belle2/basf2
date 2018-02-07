#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>../GenericB_GENSIMRECtoDST.dst.root</input>
  <output>../GenericB_Pi0s.ntup.root</output>
  <contact>Jorge Martinez-Ortega; jmartinez@fis.cinvestav.mx</contact>
</header>
"""

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdPhotons import *
from stdPi0s import *


# inputMdst('default', 'mdst_000104_prod00002265_task00000104.root')
inputMdst('default', '../GenericB_GENSIMRECtoDST.dst.root')

stdPhotons('all')
stdPhotons('loose')
stdPhotons('tight')
stdPhotons('pi0')
stdPhotons('pi0highE')

# stdPi0s('veryLoose')
# stdPi0s('loose')
stdPi0s('eff20')
stdPi0s('eff30')
stdPi0s('eff40')
stdPi0s('eff50')
stdPi0s('eff60')


# ----> NtupleMaker module
ntupleFile('GenericB_Pi0s.ntup.root')

# ----> if run on the grid
# ntupleFile('GenericB.ntup.root')


# check the pi0 list for resolution etc.

# matchMCTruth('pi0:veryLoose')
# matchMCTruth('pi0:loose')
matchMCTruth('pi0:eff20')
matchMCTruth('pi0:eff30')
matchMCTruth('pi0:eff40')
matchMCTruth('pi0:eff50')
matchMCTruth('pi0:eff60')

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
    'CustomFloats[clusterReg:clusterTiming:clusterErrorTiming:clusterE1E9]',
    'pi0 -> ^gamma ^gamma',
]


# Save the truth pi0s to ntuple
fillParticleListFromMC('gamma:gen', '')
reconstructDecay('pi0:gen -> gamma:gen gamma:gen', '')
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
ntupleTree('pi0s_eff20', 'pi0:eff20', recoToolsPi0)
ntupleTree('pi0s_eff30', 'pi0:eff30', recoToolsPi0)
ntupleTree('pi0s_eff40', 'pi0:eff40', recoToolsPi0)
ntupleTree('pi0s_eff50', 'pi0:eff50', recoToolsPi0)
ntupleTree('pi0s_eff60', 'pi0:eff60', recoToolsPi0)


# dump all event summary information
# eventtools = [
#    'EventMetaData',
#    '^B-',
#    'RecoStats',
#    '^B-',
#    'DetectorStatsRec',
#    '^B-',
#    'DetectorStatsSim',
#    '^B-',
# ]
# ntupleTree('eventtuple', '', eventtools)

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
