#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""This steering file shows how to use 'ECLChargedPIDDataAnalysis'
   module to dump ECL-related quantities in an ntuple.

Input:
    File with DST format

Output:
    Ntuple with ECL-related quantities

Usage:
    $ basf2 -i <path_to_input_file> -n <number_of_events>
            EclChargedPidAnalysis.py
"""

import basf2 as b2

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'RootInput' module
inputFile = b2.register_module('RootInput')
mainPath.add_module(inputFile)

# Register and add 'ECLChargedPIDDataAnalysis' module
eclChargedPIDDataAnalysis = b2.register_module('ECLChargedPIDDataAnalysis')
eclChargedPIDDataAnalysis.param('rootFileName',
                                'ECLChargedPIDDataAnalysis_Test.root')
mainPath.add_module(eclChargedPIDDataAnalysis)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
