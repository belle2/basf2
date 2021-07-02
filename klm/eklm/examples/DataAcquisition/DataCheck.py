#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Check collected data.

import sys
import basf2

basf2.set_log_level(basf2.LogLevel.INFO)

input = basf2.register_module('RootInput')
input.param('inputFileName', sys.argv[1])

gearbox = basf2.register_module('Gearbox')

eklmdatachecker = basf2.register_module('EKLMDataChecker')

# Create the main path and add the modules
main = basf2.create_path()
main.add_module(input)
main.add_module(gearbox)
main.add_module(eklmdatachecker)

# Process the data
basf2.process(main)

# show call statistics
print(basf2.statistics)
