#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
A test of the TDCPV variables using the small test file for running quickly
"""

import b2test_utils
from basf2 import set_random_seed, create_path, process
import modularAnalysis as ma
import vertex as vx

# make logging more reproducible by replacing some strings
b2test_utils.configure_logging_for_tests()
set_random_seed("1337")

###############################################################################
path = create_path()

# load single event from the mdst file
path.add_module('RootInput', inputFileNames=b2test_utils.require_file('analysis/tests/tdcpv-mdst.root'))

# Creates Muon particle list
ma.fillParticleList(decayString='mu+:all', cut='', path=path)

# reconstruct J/psi -> mu+ mu- decay
ma.reconstructDecay(decayString='J/psi:mumu -> mu+:all mu-:all', cut='dM<0.11', path=path)

# reconstruct Ks from standard pi+ particle list
ma.fillParticleList(decayString='pi+:all', cut='', path=path)
ma.reconstructDecay(decayString='K_S0:pipi -> pi+:all pi-:all', cut='dM<0.25', path=path)

# reconstruct B0 -> J/psi Ks decay
ma.reconstructDecay(decayString='B0:sig -> J/psi:mumu K_S0:pipi', cut='Mbc > 5.2 and abs(deltaE)<0.15', path=path)

# Does the matching between reconstructed and MC particles
ma.matchMCTruth(list_name='B0:sig', path=path)

# build the rest of the event associated to the B0
ma.buildRestOfEvent(target_list_name='B0:sig', fillWithMostLikely=True,
                    path=path)

# Vertex fit for the signal B0
vx.treeFit('B0:sig', ipConstraint=True, path=path)

# Fit Vertex of the B0 on the tag side
vx.TagV(list_name='B0:sig', MCassociation='breco', fitAlgorithm='KFit',  constraintType='tube', path=path)

tdcpv_vars = ['DeltaT', 'DeltaTErr', 'DeltaTBelle', 'TagVz', 'TagVzErr', 'mcDeltaT', 'mcDeltaTau', 'isSignal']

# Print the variables to log
ma.printVariableValues('B0:sig', var_names=tdcpv_vars, path=path)

process(path)
