#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# This steering file shows how to use EclDataAnalysis
# module to dump ECL related quantities in an ntuple
# starting from dst root file.
#
# The user should provide input and output root filnames
# as first and second argument respectively.
#
# Example steering file - 2014 Belle II Collaboration
########################################################

import basf2 as b2

# Create paths
main = b2.create_path()

# Input file
inputs = 'mdst_000544_prod00000001_task00000544.root'

simpleinput = b2.register_module('RootInput')
simpleinput.param('inputFileNames', inputs)
main.add_module(simpleinput)

# eclDataAnalysis module
eclanalysis = b2.register_module('ECLClusterAnalysis')
eclanalysis.param('rootFileName', 'EclClusterAnalysis.root')
eclanalysis.param('doTracking', 1)
main.add_module(eclanalysis)

b2.process(main)
print(b2.statistics)
