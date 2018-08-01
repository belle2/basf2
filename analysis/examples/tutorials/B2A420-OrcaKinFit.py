#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
#
# This tutorial demonstrates how to perform 4C fit with
# the orcakinfit. In this example the following decay chain:
#
# e+ e- ->  mu+ mu-
#
# is reconstructed. The 4C kinematic fit is performed on
# the non-radiative muon pairs
#
# Contributors: Torben Ferber (2017)
#
#############################################################

from basf2 import *
from modularAnalysis import *
import sys
from beamparameters import add_beamparameters

if not os.path.isfile('B2A101-Y4SEventGeneration-evtgen.root'):
    sys.exit('Required input file (B2A424-SimulateMuonPairs.root) does not exist. '
             'Please run B2A424-SimulateMuonPairs.py tutorial script first.')

set_log_level(LogLevel.WARNING)

# not necessary, but set it explicitly
beamparameters = add_beamparameters(analysis_main, "Y4S")
print_params(beamparameters)

# Input file(s).
inputMdst('default', 'B2A424-SimulateMuonPairs.root')

# use standard final state particle lists for muons
fillParticleList('mu-:z0', 'chiProb > 0.001 and p > 1.0')
fillParticleList('mu-:z0fit', 'chiProb > 0.001 and p > 1.0')

# reconstruct Z -> mu+ mu-
reconstructDecay('Z0:mm_rec -> mu+:z0 mu-:z0', '9.0 < M < 11.0')
reconstructDecay('Z0:mm_kinfit -> mu+:z0fit mu-:z0fit', '9.0 < M < 11.0')

# MC truth matching
matchMCTruth('Z0:mm_rec')
matchMCTruth('Z0:mm_kinfit')

# kinematic 4C hard fit
fitKinematic4C('Z0:mm_kinfit')


# create and fill flat Ntuple with MCTruth and kinematic information
from variableCollections import event_variables, kinematic_variables, cluster_variables, \
    track_variables, mc_variables, pid_variables, convert_to_daughter_vars, convert_to_gd_vars,\
    flight_info, mc_flight_info, vertex, mc_vertex, tag_vertex,\
    mc_tag_vertex, make_mc, momentum_uncertainty

from modularAnalysis import variablesToNTuple
rootOutputFile = 'B2A420-OrcaKinFit.root'
variablesToNTuple(filename=rootOutputFile,
                  decayString='Z0:mm_rec',
                  treename='Z0_mm_rec',
                  event_variables + kinematic_variables +
                  make_mc(kinematic_variables) + mc_variables +
                  convert_to_daughter_vars(kinematic_variables +
                                           mc_variables + momentum_uncertainty + make_mc(kinematic_variables), 0) +
                  convert_to_daughter_vars(kinematic_variables +
                                           mc_variables + momentum_uncertainty + make_mc(kinematic_variables), 1))
variablesToNTuple(filename=rootOutputFile,
                  decayString='Z0:mm_kinfit',
                  treename='Z0_mm_kinfit',
                  ['extraInfo(OrcaKinFitProb)', 'extraInfo(OrcaKinFitChi2)', 'extraInfo(OrcaKinFitErrorCode)'] +
                  event_variables + kinematic_variables + make_mc(kinematic_variables) + mc_variables +
                  convert_to_daughter_vars(kinematic_variables +
                                           mc_variables + momentum_uncertainty + make_mc(kinematic_variables), 0) +
                  convert_to_daughter_vars(kinematic_variables +
                                           mc_variables + momentum_uncertainty + make_mc(kinematic_variables), 1))

# Process the events
process(analysis_main)

# print out the summary
print(statistics)
