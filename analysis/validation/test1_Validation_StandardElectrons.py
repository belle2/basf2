#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""
<header>
  <input>../1111540100.dst.root</input>
  <output>../PIDElectrons.ntup.root</output>
  <contact>Jan Strube jan.strube@desy.de, Dmitrii Neverov dmitrii.neverov@desy.de</contact>
</header>
"""

#######################################################
#
#    B0 -> J/psi K0S(-> pi+ pi-)
#           |
#           +-> e+ e-
#
######################################################


from basf2 import *
from vertex import *
from modularAnalysis import *
from reconstruction import *
from stdFSParticles import *
from variables import variables
from ROOT import Belle2
from glob import glob

set_log_level(LogLevel.ERROR)

variables.addAlias('piExpertPID_ALL', 'pidProbabilityExpert(211, ALL)')
variables.addAlias('muExpertPID_ALL', 'pidProbabilityExpert(13, ALL)')
variables.addAlias('eExpertPID_ALL', 'pidProbabilityExpert(11, ALL)')
variables.addAlias('KExpertPID_ALL', 'pidProbabilityExpert(321, ALL)')
variables.addAlias('pExpertPID_ALL', 'pidProbabilityExpert(2212, ALL)')

input_mdst = '../1111540100.dst.root'

inputMdst('default', input_mdst)
# default: for analysis of Belle II MC samples produced
# with releases with release-00-08-00 or newer

loadStdCharged()
reconstructDecay('J/psi -> e-:all e+:all', '2.8 < M < 3.3')
vertexRave('J/psi', 0.0, "J/psi -> ^e-:all ^e+:all")

fillParticleList('K_S0', '0.3 < M < 0.7')
vertexRave('K_S0', 0.0, "K_S0 -> ^pi+:all ^pi-:all")

# Prepare the B candidates
reconstructDecay('B0 -> J/psi  K_S0', '5.2 < M < 5.4')
vertexRave('B0', 0.0, "B0 -> [J/psi -> ^e+ ^e-] ^K_S0", "iptube")

# ----> NtupleMaker module
tools = ['EventMetaData', '^B0']
tools += ['RecoStats', '^B0']
tools += ['InvMass', '^B0 -> [^J/psi -> e+:all e-:all] [^K_S0 -> pi+ pi-]']

# PID VARIABLES AND TRACK RECONSTRUCTION
tools += ['PID', 'B0 -> [J/psi -> ^e+ ^e-] [K_S0 -> ^pi+ ^pi-]']
tools += ['Kinematics', 'B0 -> [J/psi -> ^e+ ^e-] [K_S0 -> ^pi+ ^pi-]']
tools += ['CustomFloats[piExpertPID_ALL:muExpertPID_ALL:eExpertPID_ALL:KExpertPID_ALL:pExpertPID_ALL:cosTheta]',
          "B0 -> [J/psi -> ^e+ ^e-] [K_S0 -> ^pi+ ^pi-]"]


ntupleFile('../PIDElectrons.ntup.root')
ntupleTree('B0', 'B0', tools)

summaryOfLists(['J/psi', 'K_S0', 'B0'])

# ----> start processing of modules
process(analysis_main)

# ----> Print call statistics
print(statistics)
