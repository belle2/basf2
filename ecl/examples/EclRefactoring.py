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
withbg = 1  # add beam background boolean
bgfolder = '/gs/project/belle2-mcgill/users/belle2/background/campaign12'  # folder that holds beam background
bgfrac = 1.0  # fracion of nominal background
seed = 12345  # seed for random numbers
mdstfile = 'refactoring.root'  # output file

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

# add reconstruction
# tracking
add_tracking_reconstruction(main)
# post tracking

# CDC PID
CDCdEdxPID = register_module('CDCDedxPID')
main.add_module(CDCdEdxPID)

# VXD PID
VXDdEdxPID = register_module('VXDDedxPID')
main.add_module(VXDdEdxPID)

# prune tracks
add_prune_tracks(main)

# extrapolation
ext = register_module('Ext')
main.add_module(ext)

# top
top_rec = register_module('TOPReconstructor')
main.add_module(top_rec)

# arich
arich_rec = register_module('ARICHReconstructor')
main.add_module(arich_rec)

# --------------------------------------------------
# --------------------------------------------------
# ECL digit calibration
ecl_digit_calibration = register_module('ECLDigitCalibrator')
main.add_module(ecl_digit_calibration)

# ECL Shower Reconstruction
ecl_reco = register_module('ECLCRFinderAndSplitter')
main.add_module(ecl_reco)

# ECL Shower Correction
ecl_showercorrection = register_module('ECLShowerCorrector')
main.add_module(ecl_showercorrection)

# ECL Shower Calibration
ecl_showercalibration = register_module('ECLShowerCalibrator')
main.add_module(ecl_showercalibration)

# ECL Shower Shape
ecl_showershape = register_module('ECLShowerShape')
main.add_module(ecl_showershape)

# ECL covariance matrix
ecl_covariance = register_module('ECLCovarianceMatrix')
main.add_module(ecl_covariance)

# ECL finalize
ecl_finalize = register_module('ECLFinalizer')
main.add_module(ecl_finalize)

# OLD MODULES STILL USING ECLHITASSIGNMENT
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

# EKLM
eklm_rec = register_module('EKLMReconstructor')
main.add_module(eklm_rec)

# K0L reconstruction
eklm_k0l_rec = register_module('EKLMK0LReconstructor')
main.add_module(eklm_k0l_rec)

# BKLM
bklm_rec = register_module('BKLMReconstructor')
main.add_module(bklm_rec)

# K0L reconstruction
bklm_k0l_rec = register_module('BKLMK0LReconstructor')
main.add_module(bklm_k0l_rec)

# muon identifcation
muid = register_module('Muid')
main.add_module(muid)

# charged particle PID
mdstPID = register_module('MdstPID')
main.add_module(mdstPID)

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
        'ECLEventInformation'])

# Show progress of processing
progress = register_module('ProgressBar')
main.add_module(progress)

process(main)
print(statistics)
