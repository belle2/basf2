#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#######################################################
# Low-energy pi0 identification example.
######################################################

import argparse
import basf2
import modularAnalysis as ma
import pdg
import variables.collections as vc
from variables import variables

# Arguments.
parser = argparse.ArgumentParser()
parser.add_argument('input_file', default=None, help='Input file.')
parser.add_argument('output_file', default=None, help='Output file.')
parser.add_argument('--belle1', action='store_true',
                    help='Belle 1 data analysis.')
arguments = parser.parse_args()

# Create path.
analysis_path = basf2.create_path()

# Load input file.
ma.inputMdst(arguments.input_file, path=analysis_path)

# Reconstruction of photons.
gamma_list = 'gamma:all'
ma.fillParticleList(gamma_list, '', path=analysis_path)

# Reconstruction of pi0.
mass_pi0 = pdg.get('pi0').Mass()
mass_window = 0.02
mass_cut = str(mass_pi0 - mass_window) + ' < M and M < ' + \
           str(mass_pi0 + mass_window)
ma.cutAndCopyList('gamma:pi0', gamma_list, 'E > 0.02', path=analysis_path)
# Belle 1 has pi0:mdst, but reconstruct pi0 again to avoid preliminary cuts.
pi0_list = 'pi0:gamma'
ma.reconstructDecay('pi0:gamma -> gamma:pi0 gamma:pi0', mass_cut,
                    path=analysis_path)
if (arguments.belle1):
    payload_name_suffix = 'Belle1'
else:
    payload_name_suffix = 'Belle2Release5'
ma.lowEnergyPi0Identification(pi0_list, 'gamma:pi0', payload_name_suffix,
                              path=analysis_path)

# Variables.
variables.addAlias('identification', 'extraInfo(lowEnergyPi0Identification)')
pi0_vars = vc.kinematics + ['InvM', 'identification']

# Output file.
ma.variablesToNtuple('pi0:gamma', pi0_vars, filename=arguments.output_file, treename='t1', path=analysis_path)

# Progress.
analysis_path.add_module('Progress')

# Process the events.
basf2.process(analysis_path)

# Print out the summary.
print(basf2.statistics)
