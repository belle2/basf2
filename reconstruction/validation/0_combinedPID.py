#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <input>PartGunChargedStableGenSim.root</input>
  <output>CombinedPIDPerformance.root</output>
  <description>This steering script fully reconstructs particle gun events for a set of charged stable particles,
  making plots for PID performance validation</description>
  <contact>jvbennett@cmu.edu</contact>
  <interval>release</interval>
</header>
"""

import basf2
from reconstruction import add_reconstruction

main = basf2.create_path()

inputFileName = "../PartGunChargedStableGenSim.root"
main.add_module("RootInput", inputFileName=inputFileName)

add_reconstruction(main)

# run a module to generate histograms to test pid performance
pidperformance = basf2.register_module('CombinedPIDPerformance')
pidperformance.param('outputFileName', 'CombinedPIDPerformance.root')
pidperformance.param('numberOfBins', 50)
main.add_module(pidperformance)

main.add_module(register_module('ProgressBar'))

basf2.process(main)
print(basf2.statistics)
