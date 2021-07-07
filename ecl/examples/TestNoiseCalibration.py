#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Test implementation of pure CsI digitization and waveform fit

Input:
    No file is required

Output:
    Mdst file

Usage:
    $ basf2 TestNoiseCalibration.py -- --outputFileName <name>
            --elecNoise <float> --photoStatResolution <float>
            [--beamBkgPath <path_to_files>]
"""

import glob
import argparse
import basf2 as b2
from simulation import add_simulation
from reconstruction import add_reconstruction
from reconstruction import add_mdst_output


def argparser():

    parser = argparse.ArgumentParser()

    parser.add_argument('--outputFileName',
                        required=True,
                        help='Output file name')

    parser.add_argument('--elecNoise',
                        type=float,
                        required=True,
                        help='Electronics noise energy equivalent in MeV')

    parser.add_argument('--photoStatResolution',
                        type=float,
                        required=True,
                        help='Sigma for 1 MeV energy deposit')

    parser.add_argument('--beamBkgPath',
                        help='If you want to add beam background, pass this'
                        'option with path to beam background files.')
    return parser


args = argparser().parse_args()

b2.set_log_level(b2.LogLevel.ERROR)

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'EventInfoSetter' module and settings
eventInfoSetter = b2.register_module('EventInfoSetter')
eventInfoSetter.param({'evtNumList': [10],
                       'runList': [1],
                       'expList': [0]})
mainPath.add_module(eventInfoSetter)

if args.beamBkgPath:
    # Add beam background
    bgFiles = glob.glob(args.beamBkgPath + '/*.root')
    # Add simulation
    add_simulation(mainPath, bkgfiles=bgFiles, components='ECL')
else:
    add_simulation(mainPath, components='ECL')

# Add reconstruction
add_reconstruction(mainPath)

# Register and add 'ECLDigitizerPureCsI' module and settings
eclDigitizerPureCsI = b2.register_module('ECLDigitizerPureCsI')
eclDigitizerPureCsI.param('Calibration', 1)
eclDigitizerPureCsI.param('elecNoise', args.elecNoise)
eclDigitizerPureCsI.param('photostatresolution',
                          args.photoStatResolution)
eclDigitizerPureCsI.param('sigmaTrigger', 0)
eclDigitizerPureCsI.param('LastRing', 12)
mainPath.add_module(eclDigitizerPureCsI)

# Register and add 'EclCovMatrixNtuple' module and settings
eclCovMatrixNtuple = b2.register_module('EclCovMatrixNtuple')
eclCovMatrixNtuple.param('dspArrayName', 'ECLDspsPureCsI')
eclCovMatrixNtuple.param('digiArrayName', 'ECLDigitsPureCsI')
eclCovMatrixNtuple.param('outputFileName',
                         args.outputFileName)
mainPath.add_module(eclCovMatrixNtuple)

add_mdst_output(mainPath, additionalBranches=['ECLDspsPureCsI'])

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
