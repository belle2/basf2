#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../1111540100.dst.root</input>
  <output>../1111540100.ntup.root</output>
  <contact>A Morda; morda@pd.infn.it</contact>
</header>
"""

#######################################################
#
# Obtain Delta E and Mbc distribution from the decay:
#
#
#    B0 -> J/psi K0S(-> pi+ pi-)
#           |
#           +-> e+ e-
#
#
# Contributors: A. Mordà (May 2017)
#
######################################################


from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *

# load input ROOT file
inputMdst('default', '../1111540100.dst.root')
# default: for analysis of Belle II MC samples produced
# with releases with release-00-08-00 or newer

do_KS0_reco = False
loadStdCharged()


reconstructDecay('J/psi -> e-:all e+:all', '2.8 < M < 3.3')
matchMCTruth('J/psi')

if(do_KS0_reco):
    reconstructDecay('K_S0 -> pi+:all pi-:all', '0.3 < M < 0.7')
else:
    fillParticleList('K_S0', '0.3 < M < 0.7')
matchMCTruth('K_S0')

# Prepare the B candidates
reconstructDecay('B0 -> J/psi  K_S0', '5.2 < M < 5.4')
# vertexRave('B0', 0.0, "B0 -> [J/psi -> ^e+ ^e-] ^K_S0","iptube")
matchMCTruth('B0')

ntupleFile('../1111540100.ntup.root')

# ----> NtupleMaker module
tools = ['EventMetaData', 'B0']
tools += ['RecoStats', 'B0']
tools += ['Kinematics', '^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]']
tools += ['DeltaEMbc', '^B0 -> [J/psi -> e+ e-] [K_S0 -> pi+ pi-]']
tools += ['PID', 'B0 -> [J/psi -> ^e+ ^e-] [K_S0 -> ^pi+ ^pi-]']
# tools += ['Vertex', '^B0 -> [J/psi -> e+ e-] [K_S0 -> pi+ pi-]']
# tools += ['DeltaT', '^B0']


# get the rest of the event:
buildRestOfEvent('B0')

# get tag vertex ('breco' is the type of MC association)
TagV('B0', 'breco')
# buildContinuumSuppression('B0')


# ACTUAL VARIABLES FOR THE CP ASYMMETRY MEASUREMENT
tools += ['TagVertex', '^B0']

# Variables for Continuum suppression
# tools += ['ContinuumSuppression', '^B0']
# tools += ['CustomFloats[isContinuumEvent]', "^B0"]
# tools += ['CustomFloats[isSignal]', "^B0 "]

# MC INFO
tools += ['MCTruth', "^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]"]
tools += ['MCHierarchy', "^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]"]
tools += ['MCKinematics', "^B0 -> [^J/psi -> ^e+ ^e-] [^K_S0 -> ^pi+ ^pi-]"]
# tools += ['MCVertex', "^B0 -> [^J/psi -> e+ e-] [^K_S0 -> pi+ pi-]"]
# tools += ['MCTagVertex', '^B0']
# tools += ['MCDeltaT', '^B0']

ntupleTree('B0', 'B0', tools)

# dump all event summary information
# eventtools = ['EventMetaData','B0']
# eventtools += ['RecoStats','B0']
# eventtools += ['DetectorStatsRec', 'B0']
# eventtools += ['DetectorStatsSim','B0']

# ntupleTree('eventtuple', 'B0', eventtools)

# ntupleFile('1111540100.ntup.root')

summaryOfLists(['J/psi', 'K_S0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
