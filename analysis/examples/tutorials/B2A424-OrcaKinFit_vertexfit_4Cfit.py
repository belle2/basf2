#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###################################################################
# This tutorial demonstrates how to perform vc.vertexfit with RaveFit
# and four momentum constraint fit with the OrcaKinFit. In this
# example the following decay chain:
#
# Upsilon(4S) -> gamma    A
#                         |
#                         +-> u+ u-
#
# is reconstructed. The  vc.vertexfit is performed on u+ u-, and four
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
fillParticleList("gamma:sel", 'E > 0.1 and abs(formula(vc.clusterTiming/vc.clusterErrorTiming)) < 1.0')
fillParticleList("mu-:sel", 'electronID < 0.01 and chiProb > 0.001 and abs(dz) < 3 and dr < 0.1')
import pdg
pdg.add_particle('A', 9000008, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("A:sel -> mu-:sel mu+:sel", "")
reconstructDecay("A:selvc.vertex -> mu-:sel mu+:sel", "")

# Perform four momentum constraint fit using RaveFit and update the Daughters
vc.vertexRaveDaughtersUpdate("A:selvc.vertex", -1.0, constraint="iptube")

pdg.add_particle('beam', 9000009, 999., 999., 0, 0)  # name, PDG, mass, width, charge, spin
reconstructDecay("beam:sel -> A:sel gamma:sel", "")
reconstructDecay("beam:selv -> A:selvc.vertex gamma:sel", "")
reconstructDecay("beam:selv4c -> A:selvc.vertex gamma:sel", "")

# Perform four momentum constraint fit using OrcaKinFit and update the Daughters
fitKinematic4C("beam:selv4c")

# Associates the MC truth
matchMCTruth('beam:sel')
matchMCTruth('beam:selv')
matchMCTruth('beam:selv4c')

# Select variables that we want to store to ntuple
import variableCollections as vc

muvars = vc.kinematics + vc.mc_truth + mc_vc.kinematics + vc.pid + vc.momentum_uncertainty
gammavars = vc.inv_mass + vc.kinematics + mc_vc.kinematics + vc.mc_truth + vc.mc_hierarchy + vc.momentum_uncertainty
avars = vc.inv_mass + vc.kinematics + mc_vc.kinematics + vc.mc_truth + vc.mc_hierarchy + vc.momentum_uncertainty
uvars = vc.event_meta_data + vc.inv_mass + vc.kinematics + mc_vc.kinematics + vc.mc_truth + vc.mc_hierarchy + \
    vc.convert_to_all_selected_vars(muvars, 'beam -> [A -> ^mu+ ^mu-] gamma') + \
    vc.convert_to_all_selected_vars(gammavars, 'beam -> A ^gamma') + \
    vc.convert_to_all_selected_vars(avars, 'beam -> ^A gamma')

uvarsv = uvars + ['chiProb']

uvars4c = uvars + vc.wrap_list(['OrcaKinFitProb',
                                'OrcaKinFitProb',
                                'OrcaKinFitChi2',
                                'OrcaKinFitErrorCode'], 'extraInfo(variable)', "") + \
    vc.wrap_list(['VertexFitChi2',
                  'VertexFitProb'], 'daughter(1,extraInfo(variable))', "A")

# Saving variables to ntuple
from modularAnalysis import variablesToNtuple
output_file = 'B2A424-OrcaKinFit_vc.vertexfit_4Cfit.root'
variablesToNtuple('beam:selv4c', uvars4c,
                  filename=output_file, treename='beamselv4c')
variablesToNtuple('beam:selv', uvarsv,
                  filename=output_file, treename='beamselv')
variablesToNtuple('beam:sel', uvarsv,
                  filename=output_file, treename='beamsel')

#
# Process and print statistics
#

# Process the events
process(analysis_main)
# print out the summary
print(statistics)
