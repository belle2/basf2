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
# and construct pi0 veto for the signal photon. In this
# example the pi0 veto is constructed in the following
# way:
# - for each B0 -> rho gamma candidate
#     i) combine photon (gamma) used in the reconstruction
#        of the B0 candidate with all other photons found
#        in the event to form pi0 candidates
#    ii) find pi0 candidate with invariant mass closest to
#        pi0's nomina mass (closestMass)
#   iii) write value of closestMass as pi0veto extraInfo
#        flag to the B0 candidate
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
toolsB0 += ['CustomFloats[isSignal:extraInfo(pi0veto)]', '^B0']

# write out the flat ntuple
ntupleFile(rootOutputFile)
ntupleTree('b0', 'B0', toolsB0)

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
