#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#######################################################
#
# This tutorial demonstrates how to reconstruct the
# following  decay chain:
#
# B0 -> rho gamma
#       |
#       +-> pi+ pi-
#
# and construct pi0/eta veto for the signal photon. In this
# example the pi0 veto is constructed in the following
# way:
# - for each B0 -> rho gamma candidate
#     i) combine photon (gamma) used in the reconstruction
#        of the B0 candidate with all other soft photons defined
#        in analysis/scripts/modularAnalysis.py
#    ii) find pi0/eta candidate with highest pi0/eta probability
#   iii) write value of pi0/eta probability
#
# Note: This example uses the signal MC sample created in
# MC campaign 5.0, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#
######################################################

from basf2 import *
from modularAnalysis import inputMdstList
from modularAnalysis import reconstructDecay
from modularAnalysis import matchMCTruth
from modularAnalysis import analysis_main
from modularAnalysis import ntupleFile
from modularAnalysis import fillParticleList
from modularAnalysis import ntupleTree
from modularAnalysis import buildRestOfEvent
from modularAnalysis import rankByLowest
from modularAnalysis import variableToSignalSideExtraInfo
from modularAnalysis import variablesToExtraInfo
from modularAnalysis import signalSideParticleFilter
from modularAnalysis import fillSignalSideParticleList
from modularAnalysis import printVariableValues
from modularAnalysis import writePi0EtaVeto

filelist = [
    '/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000050/s00/e0000/4S/'
    'r00000/1110021000/sub00/mdst_000001_prod00000050_task00000001.root',
    '/ghi/fs01/belle2/bdata/MC/fab/sim/release-00-05-03/DBxxxxxxxx/MC5/prod00000060/s00/e0000/4S/'
    'r00000/1110022100/sub00/mdst_000989_prod00000060_task00000989.root']

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma and B0 -> K0s pi0 MC
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction.root'
inputMdstList('MC5', filelist)

fillParticleList('gamma:highE', 'E > 1.5')
fillParticleList('pi+:loose', 'abs(d0) < 0.5 and abs(z0) < 0.5 and DLLKaon > 0')

# reconstruct rho -> pi+ pi- decay
# keep only candidates with 0.6 < M(pi+pi-) < 1.0 GeV
reconstructDecay('rho0 -> pi+:loose pi-:loose', '0.6 < M < 1.0')

# reconstruct B0 -> rho0 gamma decay
# keep only candidates with Mbc > 5.2 GeV
# and -2 < Delta E < 2 GeV
reconstructDecay('B0 -> rho0 gamma:highE', '5.2 < Mbc and abs(deltaE) < 2.0')

# perform MC matching (MC truth asociation)
matchMCTruth('B0')

# build RestOfEvent (ROE) object for each B0 candidate
# ROE is required by the veto
buildRestOfEvent('B0')

# Before using the pi0etaveto you need at least the default weight files.
# Alternatively you can download them from my home directory:
# scp -r /home/belle2/otakyo/pi0etaveto
# into your workingDirectory/.
# The default working directory is '.'

# perform pi0etaveto
writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma')

# at this stage the B0 candidates should have
# extraInfo(Pi0_Prob) and extraInfo(Eta_Prob) value attached.
# extraInfo(Pi0_Prob) means pi0 probability for the B0 candidates whose gamma daughter
# extraInfo(Eta_Prob) means eta probability for the B0 candidates whose gamma daughter
# For the B0 candidates whose gamma daughter could not be combined with
# any of the remaining photons to form pi0/eta because of soft photon selection
# the extraInfo(Pi0_Prob) and extraInfo(Eta_Prob) does not exist. In these cases
# -999 will be written to the extraInfo(Pi0_Prob) branch and extraInfo(Eta_Prob) branch

# In this file, I show how to use the writePi0EtaVeto.
# You can change soft photon selection for energy and timing both. This selection is important for pi0etaveto performance.
# The default cut values are as below.
# Timing cut way is using clusterErrorTiming in VariableManager
# 0.025 GeV for forward pi0 soft photon
# 0.02 GeV for other pi0 soft photon
# 0.035 GeV for forward eta soft photon
# 0.03 GeV for other eta soft photon
# For example, you should set timecut to -2 if you want to use the energy dependent clusterTiming cut.
# writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma','', -2)
# For example, you should set timecut to 100 if you want to use the 100 ns constant clusterTiming cut.
# writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma','', 100)
# For example, you should set pi0softForward to -0.05 when you want to set forward pi0 soft photon energy to 0.02.
# writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma','', '',-0.05)
# For example, you should set pi0softForward to 0.05 when you want to set forward pi0 soft photon energy to 0.03.
# writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma','', '',0.05)

# If you train by yourself, for example, you should refer to
# B2A701-ContinuumSuppression_Input.py and
# B2A702-ContinuumSuppression_MVATrain.py.


# create and fill flat Ntuple with MCTruth and kinematic information
toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['Kinematics', '^B0 -> ^rho0 ^gamma']
toolsB0 += ['InvMass', 'B0 -> ^rho0 gamma']
toolsB0 += ['DeltaEMbc', '^B0']
toolsB0 += ['Track', 'B0 -> [rho0 -> ^pi+ ^pi-] gamma']
toolsB0 += ['Cluster', 'B0 -> rho0 ^gamma']
toolsB0 += ['MCHierarchy', 'B0 -> rho0 ^gamma']
toolsB0 += ['CustomFloats[isSignal:extraInfo(Pi0_Prob):extraInfo(Eta_Prob)]', '^B0']

# write out the flat ntuple
ntupleFile(rootOutputFile)
ntupleTree('b0', 'B0', toolsB0)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
