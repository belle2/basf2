#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# This steering file includes all modules of the
# refactored ECL code.
#
# Example steering file - 2016 Belle II Collaboration
# Author(s): Torben Ferber (ferber@physics.ubc.ca)
#
########################################################

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import add_tracking_reconstruction, add_prune_tracks, add_mdst_output

import sys
import glob

# user input
withbg = 0  # add beam background yes/no
bgfolder = ''  # folder that holds beam background
bgfrac = 1.0  # fracion of nominal background
seed = 10000  # seed for random numbers
mdstfile = 'eclrefactoring.root'  # output file

# set log level
set_log_level(LogLevel.INFO)

# fix random seed
set_random_seed(seed)

# create main path
main = create_path()

# add event infosetter
eventinfosetter = register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# add generator
evtgeninput = register_module('EvtGenInput')
main.add_module(evtgeninput)

# add default full simulation and digitization
if (withbg == 1):
    bg = glob.glob(bgfolder + '/*.root')
    add_simulation(main, bkgfiles=bg, bkgscale=bgfrac)
else:
    add_simulation(main)

# --------------------------------------------------
# --------------------------------------------------
# ECL digit calibration
ecl_digit_calibration = register_module('ECLDigitCalibrator')
main.add_module(ecl_digit_calibration)

# ECL Connected Region Finder
ecl_crfinder = register_module('ECLCRFinder')
main.add_module(ecl_crfinder)

# ECL Local Maximum Finder
ecl_lmfinder = register_module('ECLLocalMaximumFinder')
main.add_module(ecl_lmfinder)

# ECL Splitter(s)
ecl_n1 = register_module('ECLSplitterN1')
main.add_module(ecl_n1)

ecl_n2 = register_module('ECLSplitterN2')
main.add_module(ecl_n2)

# ECL Shower Correction
ecl_showercorrection = register_module('ECLShowerCorrector')
main.add_module(ecl_showercorrection)

# ECL Shower Calibration
ecl_showercalibration = register_module('ECLShowerCalibrator')
main.add_module(ecl_showercalibration)

# ECL Shower Shape
ecl_showershape = register_module('ECLShowerShape')
# ecl_showershape.set_log_level(LogLevel.DEBUG)
# ecl_showershape.set_debug_level(175)
main.add_module(ecl_showershape)

# ECL covariance matrix
ecl_covariance = register_module('ECLCovarianceMatrix')
main.add_module(ecl_covariance)

# ECL finalize
ecl_finalize = register_module('ECLFinalizer')
main.add_module(ecl_finalize)

# ECL track shower matching
ecl_track_match = register_module('ECLTrackShowerMatch')
main.add_module(ecl_track_match)

# ECL electron ID
electron_id = register_module('ECLElectronId')
main.add_module(electron_id)

# ECL MC matching
ecl_mc = register_module('MCMatcherECLClusters')
main.add_module(ecl_mc)

# --------------------------------------------------
# --------------------------------------------------

# add output file with most of the available information
add_mdst_output(
    main,
    mc=True,
    filename=mdstfile,
    additionalBranches=[
        'ECLDigits',
        'ECLCalDigits',
        'ECLConnectedRegions',
        'ECLShowers',
        'ECLLocalMaximums',
        'ECLEventInformation'])

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

process(main)
print(statistics)
