#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
#
# This steering file includes all modules of the
# ECL code.
#
# Example steering file - 2016 Belle II Collaboration
# Author(s): Torben Ferber (ferber@physics.ubc.ca)
#
########################################################

import os
from basf2 import *
from simulation import add_simulation
from reconstruction import *

import sys
import glob

# user input
withbg = 0  # add beam background yes/no
bgfolder = ''  # folder that holds beam background
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
    add_simulation(main, bkgfiles=bg)
else:
    add_simulation(main)

# add reconstruction
add_reconstruction(main)

# --------------------------------------------------
# --------------------------------------------------

# add output file with all of the available ECL information
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
