#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>PartGunChargedStableGenSim.root</input>
  <output>CombinedPIDPerformance.root</output>
  <description>This steering script fully reconstructs particle gun events for a set of charged stable particles,
  making plots for PID performance validation</description>
  <contact>jbennett@phy.olemiss.edu</contact>
  <interval>release</interval>
</header>
"""

import basf2
from reconstruction import add_reconstruction

main = basf2.create_path()

# Read input.
inputFileName = "../PartGunChargedStableGenSim.root"
main.add_module("RootInput", inputFileName=inputFileName)

# Load parameters.
main.add_module("Gearbox")
# Create geometry.
main.add_module("Geometry")

# Reconstruct events.
add_reconstruction(main)

# run a module to generate histograms to test pid performance
pidperformance = basf2.register_module('CombinedPIDPerformance')
pidperformance.param('outputFileName', 'CombinedPIDPerformance.root')
pidperformance.param('numberOfBins', 50)
main.add_module(pidperformance)

main.add_module("ProgressBar")

basf2.process(main)
print(basf2.statistics)
