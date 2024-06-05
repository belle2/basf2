#!/usr/bin/env python3

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
import b2biiConversion
import modularAnalysis as ma
import variables.collections as vc
from variables import variables

# Arguments.
parser = argparse.ArgumentParser()
parser.add_argument('--belle1', action='store_true',
                    help='Belle 1 data analysis.')
arguments = parser.parse_args()

# Create path.
analysis_path = basf2.create_path()

# Load input file.
if arguments.belle1:
    b2biiConversion.convertBelleMdstToBelleIIMdst(
        basf2.find_file('analysis/mdstBelle1_exp65_charged.root', 'validation'),
        path=analysis_path)
else:
    ma.inputMdst(filename=basf2.find_file("mdst14.root", "validation"),
                 path=analysis_path)

# Reconstruction of photons.
gamma_list = 'gamma:all'
ma.fillParticleList(gamma_list, '', path=analysis_path)

# Reconstruction of pi0.
mass_window = 0.02
mass_cut = f'abs(dM) < {mass_window}'
ma.cutAndCopyList('gamma:pi0', gamma_list, 'E > 0.02', path=analysis_path)
# Belle 1 has pi0:mdst, but reconstruct pi0 again to avoid preliminary cuts.
pi0_list = 'pi0:gamma'
ma.reconstructDecay('pi0:gamma -> gamma:pi0 gamma:pi0', mass_cut,
                    path=analysis_path)
if arguments.belle1:
    payload_name_suffix = 'Belle1'
    tag = ma.getAnalysisGlobaltagB2BII()
else:
    payload_name_suffix = 'Belle2Release5'
    tag = ma.getAnalysisGlobaltag()

# lowEnergyPi0Identification requires a payload stored in the analysis global tag
# It must be appended (or prepended) manually
basf2.conditions.append_globaltag(tag)

ma.lowEnergyPi0Identification(pi0_list, 'gamma:pi0', payload_name_suffix,
                              path=analysis_path)

# Variables.
variables.addAlias('identification', 'extraInfo(lowEnergyPi0Identification)')
pi0_vars = vc.kinematics + ['InvM', 'identification']

# Output file.
filename = 'pi0_identification.root'
ma.variablesToNtuple('pi0:gamma', pi0_vars, filename=filename, treename='t1', path=analysis_path)

# Progress.
analysis_path.add_module('Progress')

# Process the events.
basf2.process(analysis_path)

# Print out the summary.
print(basf2.statistics)
