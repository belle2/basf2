#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
from basf2 import *
from modularAnalysis import *
from stdFSParticles import *
from stdLooseFSParticles import *
from stdLightMesons import *

inputMdst('../GenericB_GENSIMRECtoDST.dst.root')
loadReconstructedParticles()
stdFSParticles()
stdLooseFSParticles()
stdLightMesons()

# ----> NtupleMaker module
ntupleFile('../GenericB.ntup.root')
##########
# Save the tracks to ntuple
recotools = [
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
ntupleTree('pituple', 'pi+:all', recotools)
# Save the truth tracks to ntuple
findMCDecay('truthPi+', 'pi+')
truthtools = [
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
ntupleTree('truthpituple', 'truthPi+', truthtools)
# Save the photons to ntuple
recotoolsGamma = [
    'EventMetaData',
    '^gamma',
    'Kinematics',
    '^gamma',
    'Track',
    '^gamma',
    'MCTruth',
    '^gamma',
    'MCKinematics',
    '^gamma',
    'MCReconstructible',
    '^gamma',
    'PID',
    '^gamma',
    ]
ntupleTree('gammatuple', 'gamma:all', recotoolsGamma)
findMCDecay('truthGamma', 'gamma')
truthtoolsGamma = [
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
ntupleTree('truthgammatuple', 'truthGamma', truthtoolsGamma)

###########
# check the pi0 list for resolution etc.
matchMCTruth('pi0:all')
pi0tools = [
    'EventMetaData',
    'pi0',
    'MCTruth',
    'pi0 -> ^gamma ^gamma',
    'MCKinematics',
    'pi0 -> ^gamma ^gamma',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    'InvMass',
    '^pi0 -> gamma gamma',
    'MCReconstructible',
    'pi0 -> ^gamma ^gamma',
    'MCHierarchy',
    'pi0 -> ^gamma ^gamma',
    ]
ntupleTree('pi0tuple', 'pi0:all', pi0tools)
findMCDecay('truthpi0', 'pi0 => gamma gamma')
pi0truthtools = [
    'EventMetaData',
    'pi0',
    'Kinematics',
    '^pi0 -> ^gamma ^gamma',
    'InvMass',
    '^pi0 -> gamma gamma',
    'MCReconstructible',
    'pi0 -> ^gamma ^gamma',
    ]
ntupleTree('truthpi0tuple', 'truthpi0', pi0truthtools)

# Check the KS0 candidates
matchMCTruth('K_S0:loose')
kstools = [
    'EventMetaData',
    'K_S0',
    'MCTruth',
    '^K_S0 -> ^pi- ^pi+',
    'Kinematics',
    '^K_S0 -> ^pi- ^pi+',
    ]
ntupleTree('kstuple', 'K_S0:loose', kstools)

##########
# check the Phi candidates
matchMCTruth('StdPhi')
phitools = [
    'EventMetaData',
    'phi',
    'MCTruth',
    '^phi -> ^K- ^K+',
    'Kinematics',
    '^phi -> ^K- ^K+',
    ]
ntupleTree('phituple', 'StdPhi', phitools)

##########
# dump all event summary information
eventtools = [
    'EventMetaData',
    'B-',
    'RecoStats',
    'B-',
    'DetectorStatsRec',
    'B-',
    'DetectorStatsSim',
    'B-',
    ]
ntupleTree('eventtuple', '', eventtools)

summaryOfLists(['StdPhi', 'K_S0:loose'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print statistics
