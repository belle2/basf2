#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# Example of Dstar reconstruction where output ntuple includes some additional variables
# from group "TOP Calibration". Input must be a cdst file.
#
# usage: basf2 cdst_recDstar.py -i <cdst_file.root> [-- (see argument parser)]
# ---------------------------------------------------------------------------------------

import basf2
import modularAnalysis as ma
from variables import variables
from vertex import raveFit
from ROOT import gSystem
import argparse

# Argument parser
ap = argparse.ArgumentParser()
ap.add_argument("--mc", help="Input file is MC", action='store_true', default=False)
ap.add_argument("--vfit", help="Do vertex fit", action='store_true', default=False)
ap.add_argument("--tag", help="Global tag (data only)", default="data_reprocessing_proc7")
ap.add_argument("--out", help="Output file", default="Dstar.root")
args = ap.parse_args()

MC = args.mc
VFit = args.vfit

if not MC:
    basf2.use_central_database(args.tag, basf2.LogLevel.WARNING)

# Load top library
gSystem.Load('libtop.so')

# Create path
main = basf2.create_path()

# Just a dummy input file, use basf2 -i option
ma.inputMdstList('default', 'Input.root', path=main)

# Particle lists
ma.fillParticleList('K-:all', '-2.0 < d0 < 2.0 and -4.0 < z0 < 4.0', path=main)
ma.fillParticleList('pi+:all', '-2.0 < d0 < 2.0 and -4.0 < z0 < 4.0', path=main)

# Reconstruct D0 -> K- pi+ decay
ma.reconstructDecay('D0:kpi -> K-:all pi+:all', '1.8 < M < 1.95', path=main)
if VFit:
    raveFit('D0:kpi', 0.001, fit_type='vertex', decay_string='', constraint='', daughtersUpdate=True, path=main)

# Reconstruct D*+ -> D0 pi+ decay
ma.reconstructDecay('D*+ -> D0:kpi pi+:all', '0.0 < Q < 0.020', path=main)
if VFit:
    raveFit('D*+', 0.001, '', 'rave', 'vertex', 'ipprofile', True, path=main)

# MC matching
if MC:
    ma.matchMCTruth('D*+', path=main)

# Output ntuple (extend the list if you need more)
variables.addAlias('isBunch', 'isTopRecBunchReconstructed')
variables.addAlias('bunchOffset', 'topRecBunchCurrentOffset')
variables.addAlias('M_D0', 'daughter(0, M)')
variables.addAlias('p_cms', 'useCMSFrame(p)')

variables.addAlias('p_K', 'daughter(0, daughter(0, p))')
variables.addAlias('flag_K', 'daughter(0, daughter(0, topFlag))')
variables.addAlias('kaonLL_K', 'daughter(0, daughter(0, topKaonLogL))')
variables.addAlias('pionLL_K', 'daughter(0, daughter(0, topPionLogL))')
variables.addAlias('slotID_K', 'daughter(0, daughter(0, topSlotID))')

variables.addAlias('p_pi', 'daughter(0, daughter(1, p))')
variables.addAlias('flag_pi', 'daughter(0, daughter(1, topFlag))')
variables.addAlias('kaonLL_pi', 'daughter(0, daughter(1, topKaonLogL))')
variables.addAlias('pionLL_pi', 'daughter(0, daughter(1, topPionLogL))')
variables.addAlias('slotID_pi', 'daughter(0, daughter(1, topSlotID))')

varlist = ['M_D0', 'Q', 'dQ', 'p_cms', 'isBunch', 'bunchOffset', 'p_K', 'p_pi',
           'kaonLL_K', 'pionLL_K', 'flag_K', 'slotID_K',
           'kaonLL_pi', 'pionLL_pi', 'flag_pi', 'slotID_pi']

ma.variablesToNtuple('D*+', varlist, 'dstar', args.out, path=main)

# Process events
basf2.process(main)

# Print statistics
print(basf2.statistics)
