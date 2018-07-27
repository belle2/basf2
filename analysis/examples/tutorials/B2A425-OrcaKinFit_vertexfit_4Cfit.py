#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
# This tutorial demonstrates how to perform vertexfit with RaveFit
# and four momentum constraint fit with the OrcaKinFit. In this
# example the following decay chain:
#
# Upsilon(4S) -> gamma    A
#                         |
#                         +-> u+ u-
#
# is reconstructed. The  vertexfit is performed on u+ u-, and four
# momentum constraint fit is performed on all final states, and the
# total four momentum is set at that of cms.
#
# Contributors: Yu Hu (July 2018)
# yu.hu@desy.de
#
####################################################################

#
# Import and mdst loading
#

from basf2 import *
from modularAnalysis import *
from stdPhotons import *
import sys
from beamparameters import add_beamparameters


# load input ROOT file
inputMdst('default', '/gpfs/group/belle2/tutorial/orcakinfit/mdst_1.root')


# Creates a list of good photon and muons
fillParticleList("gamma:sel", 'E > 0.1 and abs(formula(clusterTiming/clusterErrorTiming)) < 1.0')
fillParticleList("mu-:sel", 'electronID < 0.01 and chiProb > 0.001 and abs(dz) < 3 and dr < 0.1')
import pdg
pdg.add_particle('A', 9000008, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("A:sel -> mu-:sel mu+:sel", "")
reconstructDecay("A:selvertex -> mu-:sel mu+:sel", "")

# Perform four momentum constraint fit using RaveFit and update the Daughters
vertexRaveDaughtersUpdate("A:selvertex", -1.0, constraint="iptube")

pdg.add_particle('beam', 9000009, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("beam:sel -> A:sel gamma:sel", "")
reconstructDecay("beam:selv -> A:selvertex gamma:sel", "")
reconstructDecay("beam:selv4c -> A:selvertex gamma:sel", "")

# Perform four momentum constraint fit using OrcaKinFit and update the Daughters
fitKinematic4C("beam:selv4c")

# Associates the MC truth
matchMCTruth('beam:sel')
matchMCTruth('beam:selv')
matchMCTruth('beam:selv4c')

# here you can put whatever you like....
toolsD0v4c = ['EventMetaData', '^beam:selv4c']
toolsD0v4c += ['InvMass', '^beam:selv4c -> ^A ^gamma']
toolsD0v4c += ['Kinematics', '^beam:selv4c -> [^A -> ^mu- ^mu+] ^gamma']
toolsD0v4c += ['MCKinematics', '^beam:selv4c -> [^A -> ^mu- ^mu+] ^gamma']
toolsD0v4c += ['PID', 'beam:selv4c -> [A -> ^mu+ ^mu-] gamma']
toolsD0v4c += ['MCTruth', '^beam:selv4c ->  [^A -> ^mu- ^mu+] ^gamma']
toolsD0v4c += ['MCHierarchy', '^beam:selv4c -> ^A ^gamma']
toolsD0v4c += ['CustomFloats[E:px:py:pz:E_uncertainty:pxErr:pyErr:pzErr]', '^beam:selv4c ->  [^A -> ^mu- ^mu+] ^gamma']
toolsD0v4c += ['CustomFloats[extraInfo(VertexFitChi2)]', 'beam:selv4c -> ^A gamma']
toolsD0v4c += ['CustomFloats[extraInfo(VertexFitProb)]', 'beam:selv4c -> ^A gamma']
toolsD0v4c += ['CustomFloats[extraInfo(OrcaKinFitProb)]', '^beam:selv4c']
toolsD0v4c += ['CustomFloats[extraInfo(OrcaKinFitChi2)]', '^beam:selv4c']
toolsD0v4c += ['CustomFloats[extraInfo(OrcaKinFitErrorCode)]', '^beam:selv4c']


toolsD0v = ['EventMetaData', '^beam:selv']
toolsD0v += ['InvMass', '^beam:selv -> ^A ^gamma']
toolsD0v += ['Kinematics', '^beam:selv -> [^A -> ^mu+ ^mu-] ^gamma']
toolsD0v += ['MCKinematics', '^beam:selv -> [^A -> ^mu+ ^mu-] ^gamma']
toolsD0v += ['PID', 'beam:selv -> [A -> ^mu+ ^mu-] gamma']
toolsD0v += ['MCTruth', '^beam:selv ->  [^A -> ^mu+ ^mu-] gamma']
toolsD0v += ['MCHierarchy', '^beam:selv -> ^A ^gamma']
toolsD0v += ['CustomFloats[chiProb]', '^beam:selv']
toolsD0v += ['CustomFloats[E:px:py:pz:E_uncertainty:pxErr:pyErr:pzErr]', '^beam:selv ->  [^A -> ^mu- ^mu+] ^gamma']

toolsD0 = ['EventMetaData', '^beam:sel']
toolsD0 += ['InvMass', '^beam:sel -> ^A ^gamma']
toolsD0 += ['Kinematics', '^beam:sel -> [^A -> ^mu+ ^mu-] ^gamma']
toolsD0 += ['MCKinematics', '^beam:sel -> [^A -> ^mu+ ^mu-] ^gamma']
toolsD0 += ['PID', 'beam:sel -> [A -> ^mu+ ^mu-] gamma']
toolsD0 += ['MCTruth', '^beam:sel ->  [^A -> ^mu+ ^mu-] gamma']
toolsD0 += ['MCHierarchy', '^beam:sel -> ^A ^gamma']
toolsD0 += ['CustomFloats[chiProb]', '^beam:sel']
toolsD0 += ['CustomFloats[E:px:py:pz:E_uncertainty:pxErr:pyErr:pzErr]', '^beam:sel ->  [^A -> ^mu- ^mu+] ^gamma']

ntupleFile('B2A425-OrcaKinFit_vertexfit_4Cfit.root')
ntupleTree('beamselv4c', 'beam:selv4c', toolsD0v4c)
ntupleTree('beamselv', 'beam:selv', toolsD0v)
ntupleTree('beamsel', 'beam:sel', toolsD0)

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
