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

import glob
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output

# user input
withbg = 0  # add beam background yes/no
bgfolder = ''  # folder that holds beam background
seed = 10000  # seed for random numbers
mdstfile = 'eclrefactoring.root'  # output file

# set log level
b2.set_log_level(b2.LogLevel.INFO)

# fix random seed
b2.set_random_seed(seed)

# create main path
main = b2.create_path()

# add event infosetter
eventinfosetter = b2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# add generator
evtgeninput = b2.register_module('EvtGenInput')
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
        'ECLLocalMaximums'])

# Show progress of processing
progress = b2.register_module('ProgressBar')
main.add_module(progress)

b2.process(main)
print(b2.statistics)
