#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# KLM alignment: simulation.

import sys
import basf2
from simulation import add_simulation

# Use local database.
basf2.conditions.append_testing_payloads('localdb/database.txt')

# Create path.
main = basf2.create_path()

# Input.
main.add_module('RootInput', inputFileName=sys.argv[1])

# Simulation.
add_simulation(main)

# Output.
main.add_module('RootOutput', outputFileName=sys.argv[2])

# Progress.
main.add_module('Progress')

# Processing.
basf2.process(main)

# Print call statistics
print(basf2.statistics)
