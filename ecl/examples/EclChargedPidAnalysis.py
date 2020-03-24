#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

__authors__ = ['Caitlin MacQueen', 'Marco Milesi']
__copyright__ = 'Copyright 2018 - Belle II Collaboration'
__maintainer__ = 'Abtin Narimani Charan'
__email__ = 'abtin.narimani.charan@desy.de'

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
