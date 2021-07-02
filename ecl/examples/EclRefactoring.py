#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file includes all modules of the
   ECL code.

Usage:
    basf2 EclRefactoring.py [-- --bkgDirectory <path_to_files>]
"""

import glob
import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output

__author__ = ['Torben Ferber', 'Abtin Narimani Charan']
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'


def argparser():

    parser = argparse.ArgumentParser()

    parser.add_argument('--bkgDirectory',
                        help='Path to beam background directory'
                        'If you want to add beam background, pass this option.')
    return parser


args = argparser().parse_args()

# Set log level
b2.set_log_level(b2.LogLevel.INFO)

# Fix random seed
b2.set_random_seed(123456)

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'EventInfoSetter' module
eventInfoSetter = b2.register_module('EventInfoSetter')
mainPath.add_module(eventInfoSetter)

# Register and add 'EvtGenInput' module
evtGenInput = b2.register_module('EvtGenInput')
mainPath.add_module(evtGenInput)

# Add default full simulation and digitization
if args.bkgDirectory:
    # Add beam background
    bgFiles = glob.glob(args.bkgDirectory + '/*.root')
    # Add simulation
    add_simulation(mainPath, bkgfiles=bgFiles)
else:
    add_simulation(mainPath)

# Add reconstruction
add_reconstruction(mainPath)

# Add output mdst file with all of the available ECL information
add_mdst_output(
    mainPath,
    mc=True,
    filename='ecl_refactoring.mdst.root',
    additionalBranches=[
        'ECLDigits',
        'ECLCalDigits',
        'ECLConnectedRegions',
        'ECLShowers',
        'ECLLocalMaximums'])

# Show progress of processing
progressBar = b2.register_module('ProgressBar')
mainPath.add_module(progressBar)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
