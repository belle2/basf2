#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import logging, LogLevel, create_path, process
# Don't show all the info messages
logging.log_level = LogLevel.ERROR

# create path
main = create_path()

# We need to process one event so we need the EventInfoSetter
main.add_module("EventInfoSetter", evtNumList=[1])

# We need the geometry parameters
main.add_module("Gearbox")

# as well as the geometry. We assign regions to each creator which allows to
# split the material budget by region instead of material. Also, we restrict
# the geometry to certain sub detectors only, in this case just the central
# beampipe, the PXD and the SVD.
geometry = main.add_module("Geometry", logLevel=LogLevel.INFO, assignRegions=True,
                           components=['BeamPipe', 'PXD', 'SVD'])

# MaterialScan uses the Geant4 setup which is created by the FullSim module so
# we need this as well
main.add_module('FullSim')

# And finally the MaterialScan module
materialscan = main.add_module("MaterialScan", logLevel=LogLevel.INFO)

# Create a detailed Materialbudget for the beampipe
# 1) 100x100 raster starting from IP looking at the acceptance
# 2) 200x200 raster of the ZX plane between Z=-50 to 60 cm and X=-16 to
#    16 cm, starting at Y=-16cm and scanning the next 32cm along Y

materialscan.param({
    # Filename for output File
    'Filename': 'MaterialScan.root',
    # Do Spherical scan?
    'spherical': True,
    # Origin of the spherical scan
    'spherical.origin': [0, 0, 0],
    # Number of steps in theta
    'spherical.nTheta': 100,
    # Minimal theta value
    'spherical.minTheta': 17,
    # Maximal theta value
    'spherical.maxTheta': 150,
    # If set to true, the scan will be done uniform in cos(theta)
    'spherical.cosTheta': True,
    # Number of steps in theta
    'spherical.nPhi': 100,
    # Minimal theta value
    'spherical.minPhi': 0,
    # Maximal theta value
    'spherical.maxPhi': 360,
    # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
    # will stop the ray after reaching the given distance from the start point
    'spherical.maxDepth': 0,
    # Split output by Material names instead of by Region
    'spherical.splitByMaterials': False,
    # Specify the names of Materials to ignore in the scan.
    # Default is Air and Vacuum
    'spherical.ignored': ['Air', 'Vacuum', 'G4_AIR', 'ColdAir'],
    # Do Planar scan?
    'planar': True,
    # Name of the plane for scanning.  One of 'XY', 'XZ', 'YX', 'YZ', 'ZX',
    # 'ZY' or 'custom' to define the plane directly
    'planar.plane': 'custom',
    # Define a custom plane for the scan, only used if planar.plane='custom'.
    # This is a list of 9 values where the first three values specify the
    # origin, the second 3 values specify the first axis and the last 3 values
    # specify the second axis. In this case we scan the ZX plane but the origin
    # is not at the IP at y=-1 to create a symmetric cut view in |y|< 1 cm
    'planar.custom': [
        0, -16, 0,  # Origin
        0,   0, 1,  # First axis
        1,   0, 0,  # Second axis
    ],
    # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
    # will stop the ray after reaching the given distance from the start point
    'planar.maxDepth': 32,
    # Number of steps along the first axis
    'planar.nU': 200,
    # Minimum value for the first axis
    'planar.minU': -50.0,
    # Maximum value for the first axis
    'planar.maxU': 60,
    # Number of steps along the second axis
    'planar.nV': 200,
    # Minimum value for the second axis
    'planar.minV': -16,
    # Maximum value for the second axis
    'planar.maxV': 16,
    # Split output by Material names insted of by Region
    'planar.splitByMaterials': True,
    # Specify the names of Materials to ignore in the scan.
    # Default is Air and Vacuum
    'planar.ignored': ['Air', 'G4_AIR', 'ColdAir'],
})

# Do the MaterialScan, can take some time depending on the number of steps
process(main)
