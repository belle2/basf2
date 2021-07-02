#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file tests the 'ECLChargedPID' module.

Input:
    It must run on DST files, or reduced DSTs containing
    ECLShowers, Tracks, and all relevant relational containers:

        - Tracks
        - TrackFitResults
        - ECLClusters
        - ECLShowers
        - TracksToECLClusters
        - TracksToECLShowers
        - ECLClustersToECLShowers
        - ECLConnectedRegions
        - ECLPidLikelihoods
        - PIDLikelihoods
        - TracksToECLPidLikelihoods
        - TracksToPIDLikelihoods
Usage:
    $ basf2 -i <path_to_input_file> -n <number_of_events>
            EclChargedPidModuleTest.py
"""

import basf2 as b2

__author__ = 'Marco Milesi'
__copyright__ = 'Copyright 2019 - Belle II Collaboration'
__maintainer__ = 'Marco Milesi'
__email__ = 'marco.milesi@unimelb.edu.au'

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'RootInput' module to read input DST file.
inputFile = b2.register_module('RootInput')
mainPath.add_module(inputFile)

# Register and add 'ECLChargedPID' module
eclChargedPID = b2.register_module('ECLChargedPID')
mainPath.add_module(eclChargedPID)

# Set debug options for 'ECLChargedPID' module.
eclChargedPID.logging.log_level = b2.LogLevel.DEBUG
eclChargedPID.logging.debug_level = 20

"""Register and add 'PrintCollections' module.
   This prints the data model objects in the store.
"""
printCollections = b2.register_module('PrintCollections')
mainPath.add_module(printCollections)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
