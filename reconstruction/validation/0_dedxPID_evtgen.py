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
  <output>EvtGenSimRec_dedx.root</output>
  <description>Generates dE/dx debug data (DedxTracks) for testing</description>
  <contact>jkumar@andrew.cmu.edu</contact>
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

# enable debug output for the module added by add_reconstruction()
for m in main.modules():
    if m.name() == 'CDCDedxPID':
        m.param('enableDebugOutput', True)
    if m.name() == 'VXDDedxPID':
        m.param('enableDebugOutput', True)
        # m.param('usePXD', True)


output = basf2.register_module('RootOutput')
output.param('outputFileName', '../EvtGenSimRec_dedx.root')
# let's keep this small
output.param('branchNames', ['CDCDedxLikelihoods', 'CDCDedxTracks', 'VXDDedxLikelihoods', 'VXDDedxTracks', 'EventMetaData'])
main.add_module(output)

main.add_module("ProgressBar")

basf2.process(main)
print(basf2.statistics)
