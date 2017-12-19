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
# MC campaign 9, therefore it can be ran only on KEKCC computers.
#
# Contributors: A. Zupanc (June 2014)
#               K. Ota (Oct 2017)
#               I. Komarov (December 2017)
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
    '/ghi/fs01/belle2/bdata/MC/release-00-09-01/DB00000276/MC9/prod00002326/e0000/4S/'
    'r00000/1110021010/sub00/mdst_000001_prod00002326_task00000001.root']

# Run B0 -> rho gamma reconstruction over B0 -> rho gamma and B0 -> K0s pi0 MC
rootOutputFile = 'B2A304-B02RhoGamma-Reconstruction.root'
inputMdstList('default', filelist)

fillParticleList('gamma:highE', 'E > 1.5')
fillParticleList('pi+:loose', 'abs(d0) < 0.5 and abs(z0) < 0.5 and pionID > 0.002')

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

# perform pi0/eta veto
writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma', workingDirectory='./pi0etaveto')

# at this stage the B0 candidates should have
# extraInfo(Pi0_Prob) and extraInfo(Eta_Prob) value attached.
# extraInfo(Pi0_Prob) means pi0 probability for the B0 candidates whose gamma daughter.
# extraInfo(Eta_Prob) means eta probability for the B0 candidates whose gamma daughter.
# For the B0 candidates whose gamma daughter could not be combined with
# any of the remaining photons to form pi0/eta because of soft photon selection
# the extraInfo(Pi0_Prob) and extraInfo(Eta_Prob) does not exist. In these cases
# -999 will be written to the extraInfo(Pi0_Prob) branch and extraInfo(Eta_Prob) branch.
# You can change extraInfo names of pi0/eta probability by setting pi0vetoname and etavetoname parameters. For example,
# writePi0EtaVeto('B0', 'B0 -> rho0 ^gamma', workingDirectory='./pi0etaveto', pi0vetoname='Pi0_Prob2', etavetoname='Eta_Prob2')

# You need at least the default weight files: pi0veto.root and etaveto.root for writePi0EtaVeto.
# The default files are optimised by MC campaign 9.
# If you don't have weight files in your workingDirectory,
# these files are downloaded from database to your workingDirectory automatically.
# The default workingDirectory is '.'
# You can also download them from following directory in KEKCC:
# /gpfs/group/belle2/users/akimasa/pi0etaveto
# If you train by yourself, you should refer to
# B2A701-ContinuumSuppression_Input.py
# B2A702-ContinuumSuppression_MVATrain.py


# You can also do a simple veto using delta mass ranking as below.

# VETO starts here
# ----------------

# Create a new path (called ROE path) which will be executed for
# each ROE in an event.
# Note that ROE exists for each B0 candidate, so when we loop
# over each ROE, we effectively loop over signal B0 candidates

roe_path = create_path()

# The ROE objects might in general be related to Particle from multiple
# particle lists therfore we need to check if the current ROE object
# is related to the Particle from our signal decay. If it is not
# the execution of roe_path will be finished (by starting empty,
# dead end path). Note that in this example this x-check is not
# neccessary, but is anyway added for sake of completness
deadEndPath = create_path()

# Note again: all actions (modules) included in roe_path will be
# executed for each ROE in the event
# First we check that the current ROE is related to B0 candidate
signalSideParticleFilter('B0', '', roe_path, deadEndPath)

# create and fill gamma ParticleList that will contain
# all photons found in ROE (not used to reconstruct current B0 candidate)
# The photons need to have energy above 50 MeV to be considered
# (one can add any cut)
fillParticleList('gamma:roe', 'isInRestOfEvent == 1 and E > 0.050', path=roe_path)

# in order to be able to use modularAnalysis functions (reconstructDecay in particular)
# we need a ParticleList containg the photon candidate used to reconstruct the
# current B meson as well
# The DecayString is used to specify the selected particle (^)
fillSignalSideParticleList('gamma:sig', 'B0 -> rho0 ^gamma', roe_path)

# make combinations of signal photon candidates with all photons from ROE
# keep only combinations in given invariant mass range
reconstructDecay('pi0:veto -> gamma:sig gamma:roe', '0.080 < M < 0.200', path=roe_path)

# at this point one could use all features provided by the analysis software
# to make the veto as effective as possible. For example, one can perform truth
# matching, training/applying TMVA classifier, save pi0 candidates with ntuple
# maker for offline analysis/study.

# in this example the variable, which is used to veto pi0 is very simple:
# invariant mass of pi0 that is closest to the pi0's nominal mass
# Therfore, we just simply rank pi0 candidates according to their distance
# from nominal mass (dM variable) and keep only the best candidate
rankByLowest('pi0:veto', 'abs(dM)', 1, path=roe_path)

# write the invariant mass of the best pi0 candidate to the current B0
# candidate as the 'pi0veto' extraInfo
variableToSignalSideExtraInfo('pi0:veto', {'M': 'pi0veto'}, path=roe_path)

# execute roe_path for each RestOfEvent in the event
analysis_main.for_each('RestOfEvent', 'RestOfEvents', roe_path)

# VETO ends here
# ----------------

# we're now out of the ROE path
# at this stage the B0 candidates should have
# extraInfo(pi0veto) value attached. For the B0
# candidates whose gamma daughter could not be combined with
# any of the remaining photons to form pi0 within given mass
# range the extraInfo(pi0veto) does not exist. In these cases
# -999 will be written to the extraInfo(pi0veto) branch

# create and fill flat Ntuple with MCTruth and kinematic information
toolsB0 = ['EventMetaData', '^B0']
toolsB0 += ['Kinematics', '^B0 -> ^rho0 ^gamma']
toolsB0 += ['InvMass', 'B0 -> ^rho0 gamma']
toolsB0 += ['DeltaEMbc', '^B0']
toolsB0 += ['Track', 'B0 -> [rho0 -> ^pi+ ^pi-] gamma']
toolsB0 += ['Cluster', 'B0 -> rho0 ^gamma']
toolsB0 += ['MCHierarchy', 'B0 -> rho0 ^gamma']
toolsB0 += ['CustomFloats[isSignal:extraInfo(Pi0_Prob):extraInfo(Eta_Prob):extraInfo(pi0veto)]', '^B0']

# write out the flat ntuple
ntupleFile(rootOutputFile)
ntupleTree('b0', 'B0', toolsB0)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
