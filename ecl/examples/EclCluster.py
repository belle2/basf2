#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""This steering file shows how to use 'ECLClusterAnalysis'
   module to dump ECL-related quantities in an ntuple.

Input:
    File with mdst format

Output:
    Ntuple with ECL cluster quantities

Usage:
    $ basf2 -i <path_to_input_file> -n <number_of_events>
            EclCluster.py
"""

import basf2 as b2

__authors__ = ['Benjamin Oberhof', 'Frank Meier']
__copyright__ = 'Copyright 2018 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'

# Create path. Register necessary modules to this path.
mainPath = b2.create_path()

# Register and add 'RootInput' module
inputFile = b2.register_module('RootInput')
mainPath.add_module(inputFile)

# Register and add 'ECLClusterAnalysis' module and settings
eclClusterAnalysis = b2.register_module('ECLClusterAnalysis')
eclClusterAnalysis.param('rootFileName',
                         'EclClusterAnalysis_Test.root')
eclClusterAnalysis.param('doTracking', 1)
mainPath.add_module(eclClusterAnalysis)

# Process the events and print call statistics
mainPath.add_module('Progress')
b2.process(mainPath)
print(b2.statistics)
