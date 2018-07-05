#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../1111440100.dst.root</input>
  <output>../1111440100.ntup.root</output>
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
#           +-> mu+ mu-
#
#
# Contributors: A. Mordà (May 2017)
#
######################################################


from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdCharged import *
from variables import variables
from ROOT import Belle2
from glob import glob

set_log_level(LogLevel.ERROR)


input_mdst = '../1111440100.dst.root'

inputMdst('default', input_mdst)
# default: for analysis of Belle II MC samples produced
# with releases with release-00-08-00 or newer


loadStdCharged()
stdLooseMu()
cutAndCopyList('mu+:Jp', 'mu+:loose', '')

reconstructDecay('J/psi -> mu-:Jp mu+:Jp', '2.8 < M < 3.3')
vertexRave('J/psi', 0.0, "J/psi -> ^mu-:Jp ^mu+:Jp")
matchMCTruth('J/psi')

fillParticleList('K_S0', '0.3 < M < 0.7')
vertexRave('K_S0', 0.0, "K_S0 -> ^pi+:all ^pi-:all")
matchMCTruth('K_S0')

# Prepare the B candidates
reconstructDecay('B0 -> J/psi  K_S0', '5.2 < M < 5.4')
vertexRave('B0', 0.0, "B0 -> [J/psi -> ^mu+ ^mu-] ^K_S0", "iptube")
matchMCTruth('B0')


# get the rest of the event:
buildRestOfEvent('B0')

# get tag vertex ('breco' is the type of MC association)
TagV('B0', 'breco', confidenceLevel=1.E-3)

# get continuum suppression (needed for flavor tagging)
cleanMask = ('cleanMask', 'useCMSFrame(p)<=3.2', 'p >= 0.05 and useCMSFrame(p)<=3.2')
appendROEMasks('B0', [cleanMask])
buildContinuumSuppression('B0', 'cleanMask')

# ----> NtupleMaker module
tools = ['EventMetaData', '^B0']
tools += ['RecoStats', '^B0']

# KINEMATIC VARIABLES
tools += ['Kinematics', '^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]']
tools += ['DeltaEMbc', '^B0 -> [J/psi -> mu+:Jp mu-:Jp] [K_S0 -> pi+ pi-]']
tools += ['InvMass', '^B0 -> [^J/psi -> mu+:Jp mu-:Jp] [^K_S0 -> pi+ pi-]']


# VARIABLES FOR REST OF EVENTS (ROE)
tools += ['ROEMultiplicities', "^B0"]

# PID VARIABLES AND TRACK RECONSTRUCTION
tools += ['PID', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
tools += ['Track', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']
tools += ['TrackHits', 'B0 -> [J/psi -> ^mu+ ^mu-] [K_S0 -> ^pi+ ^pi-]']


# ACTUAL VARIABLES FOR THE CP ASYMMETRY MEASUREMENT
tools += ['Vertex', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
tools += ['DeltaT', '^B0']
tools += ['TagVertex', '^B0']


# MC INFO
tools += ['ContinuumSuppression', '^B0']
tools += ['CustomFloats[isSignal]', '^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]']
tools += ['CustomFloats[isContinuumEvent]', "^B0"]
tools += ['MCTruth', "^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]"]
tools += ['MCHierarchy', "^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]"]
tools += ['MCKinematics', "^B0 -> [^J/psi -> ^mu+ ^mu-] [^K_S0 -> ^pi+ ^pi-]"]
tools += ['MCVertex', "^B0 -> [^J/psi -> mu+ mu-] [^K_S0 -> pi+ pi-]"]
tools += ['MCTagVertex', '^B0']
tools += ['MCDeltaT', '^B0']

ntupleFile('../1111440100.ntup.root')
ntupleTree('B0', 'B0', tools)


summaryOfLists(['J/psi', 'K_S0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
