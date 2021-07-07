#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# Simple steering file to demonstrate how to run testbeam
# Before running, you need to set upgrade global tag
# environment variable (BELLE2_VTX_UPGRADE_GT).
#
# Usage: python3 testbeam_example.py (-n 100000 -angle 0)
#############################################################

from vtx import get_upgrade_globaltag
import basf2 as b2
from VTXOutputDumper import VTXOutputDumper

import argparse
parser = argparse.ArgumentParser(description="Perform runs of test beam")
parser.add_argument('-n', default=100000, type=int, help='Number of events to generate')
parser.add_argument('-angle', default=0.0, type=float, help='Angle (deg) for the particleGun (in phi)')
parser.add_argument('-forceCoG', default=False, action='store_true', help="Use only(!) center of gravity based position finding")
args = parser.parse_args()

b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())


# Number of events to generate
num_events = args.n

# Angle for the particleGun (in phi)
angle = args.angle

# Output file name
outputfile = f"testbeam_{angle}deg.root"

# create path
main = b2.create_path()

main.add_module("EventInfoSetter", evtNumList=num_events)

main.add_module('ParticleGun', pdgCodes=[11],
                momentumGeneration="fixed", momentumParams=[5.2],
                thetaGeneration="fixed", thetaParams=[90.],
                phiGeneration="fixed", phiParams=[angle],
                # normal distributed vertex generation
                vertexGeneration='normal',
                xVertexParams=[-0.02, 0.0],
                yVertexParams=[0.28, 0.3],
                zVertexParams=[0.95, 0.3]
                )

main.add_module('Gearbox')
main.add_module('Geometry')

# G4 simulation
main.add_module('FullSim')

# Digitizer
main.add_module('VTXDigitizer')

# Activate cluster shape correction
if not args.forceCoG:
    main.add_module("ActivateVTXClusterPositionEstimator")

# Clusturizer
main.add_module('VTXClusterizer')

# Make the tree
output = VTXOutputDumper(outputfile)
main.add_module(output)

# process events and print call statistics
main.add_module('Progress')
b2.process(main)
print(b2.statistics)
