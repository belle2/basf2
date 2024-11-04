#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ------------------------------------------------------------------------------------------
# Example of Dstar reconstruction where output ntuple includes some additional variables
# from group "TOP Calibration". Input must be a cdst file.
#
# One can print the "TOP Calibration" variables with basf2 top/examples/printTOPVariables.py
#
# usage: basf2 cdst_recDstar.py -i <cdst_file.root> [-- (see argument parser)]
# ------------------------------------------------------------------------------------------

import basf2
from reconstruction import prepare_user_cdst_analysis
import modularAnalysis as ma
from variables import variables
from vertex import treeFit
from ROOT import gSystem
import argparse
import sys

# Argument parser
ap = argparse.ArgumentParser()
ap.add_argument("--mc", help="Input file is MC", action='store_true', default=False)
ap.add_argument("--exp", help="Experiment number, if input file is real data", default=0)
ap.add_argument("--vfit", help="Do vertex fit", action='store_true', default=False)
ap.add_argument("--out", help="Output file", default="Dstar.root")
args = ap.parse_args()

MC = args.mc
VFit = args.vfit

# Global tags if running on real data
# *****************************************************************************************************
# note: The patching global tags and their order can be bucket number and basf2 version dependent.
#       Given below is what's been tested to work with cdst files of bucket 16 calibration on release-7
# *****************************************************************************************************
if not MC:
    if args.exp == 0:
        basf2.B2ERROR("Experiment number is not given. Please, provide it with --exp")
        sys.exit()
    basf2.conditions.override_globaltags()
    basf2.conditions.append_globaltag('patch_main_release-07_noTOP')
    if int(args.exp) < 20:
        basf2.conditions.append_globaltag('data_reprocessing_proc13')  # experiments 7 - 18
    else:
        basf2.conditions.append_globaltag('data_reprocessing_prompt')  # experiments 20 - 26
    basf2.conditions.append_globaltag('online')

# Load top library
gSystem.Load('libtop.so')

# Create path
main = basf2.create_path()

# Just a dummy input file, use basf2 -i option
ma.inputMdstList('Input.root', path=main)

# Run unpackers and post-tracking reconstruction
prepare_user_cdst_analysis(main)

# Particle lists
ma.fillParticleList('K-:All', '-2.0 < d0 < 2.0 and -4.0 < z0 < 4.0', path=main)
ma.fillParticleList('pi+:All', '-2.0 < d0 < 2.0 and -4.0 < z0 < 4.0', path=main)

# Reconstruct D0 -> K- pi+ decay
ma.reconstructDecay('D0:kpi -> K-:All pi+:All', '1.8 < M < 1.95', path=main)

# Reconstruct D*+ -> D0 pi+ decay
ma.reconstructDecay('D*+ -> D0:kpi pi+:All', '0.0 < Q < 0.020', path=main)

# Vertex fit
if VFit:
    treeFit('D*+', ipConstraint=True, path=main)

# MC matching
if MC:
    ma.matchMCTruth('D*+', path=main)

# Output ntuple (extend the list if you need more)
variables.addAlias('isBunch', 'topBunchIsReconstructed')
variables.addAlias('bunchOffset', 'topBunchOffset')
variables.addAlias('M_D0', 'daughter(0, M)')
variables.addAlias('p_cms', 'useCMSFrame(p)')

variables.addAlias('p_K', 'daughter(0, daughter(0, p))')
variables.addAlias('flag_K', 'daughter(0, daughter(0, topLogLFlag))')
variables.addAlias('kaonLL_K', 'daughter(0, daughter(0, topLogLKaon))')
variables.addAlias('pionLL_K', 'daughter(0, daughter(0, topLogLPion))')
variables.addAlias('slotID_K', 'daughter(0, daughter(0, topSlotID))')

variables.addAlias('p_pi', 'daughter(0, daughter(1, p))')
variables.addAlias('flag_pi', 'daughter(0, daughter(1, topLogLFlag))')
variables.addAlias('kaonLL_pi', 'daughter(0, daughter(1, topLogLKaon))')
variables.addAlias('pionLL_pi', 'daughter(0, daughter(1, topLogLPion))')
variables.addAlias('slotID_pi', 'daughter(0, daughter(1, topSlotID))')

varlist = ['M_D0', 'Q', 'dQ', 'p_cms', 'isBunch', 'bunchOffset', 'p_K', 'p_pi',
           'kaonLL_K', 'pionLL_K', 'flag_K', 'slotID_K',
           'kaonLL_pi', 'pionLL_pi', 'flag_pi', 'slotID_pi']

ma.variablesToNtuple('D*+', varlist, 'dstar', args.out, path=main)

# Process events
basf2.process(main)

# Print statistics
print(basf2.statistics)
