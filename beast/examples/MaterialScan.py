#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import logging, LogLevel, register_module, create_path, process
# Don't show all the info messages
logging.log_level = LogLevel.ERROR

# We need to process one event so we need the EventInfoSetter
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'runList': [1], 'evtNumList': [1]})

# We need the geometry parameters
gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase1.xml')
# as well as the geometry
geometry = register_module('Geometry')
geometry.set_log_level(LogLevel.INFO)
# Restrict Geometry to certain components, in this case only PXD and SVD
# geometry.param('components', ['PH1BPIPE'])

# MaterialScan uses the Geant4 setup which is created by the FullSim module so
# we need this as well
simulation = register_module('FullSim')

# And finally the MaterialScan module
materialscan = register_module('MaterialScan')
materialscan.set_log_level(LogLevel.INFO)

# Create a detailed Materialbudget for the beampipe
# 1) 1000x1000 raster starting from IP looking at the acceptance
# 2) 2000x2000 raster of the ZX plane between Z=-50 to 60 cm and X=-16 to
#    16 cm, starting at Y=-16cm and scanning the next 32cm along Y

materialscan.param({  # Filename for output File
                      # Do Spherical scan?
                      # Origin of the spherical scan
                      # Number of steps in theta
                      # Minimal theta value
                      # Maximal theta value
                      # If set to true, the scan will be done uniform in cos(theta)
                      # Number of steps in theta
                      # Minimal theta value
                      # Maximal theta value
                      # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
                      # will stop the ray after reaching the given distance from the start point
                      # Split output by Material names instead of by Region
                      # Specify the names of Materials to ignore in the scan.
                      # Default is Air and Vacuum
                      # Do Planar scan?
                      # Name of the plane for scanning.  One of 'XY', 'XZ', 'YX', 'YZ', 'ZX',
                      # 'ZY' or 'custom' to define the plane directly
                      # Define a custom plane for the scan, only used if planar.plane='custom'.
                      # This is a list of 9 values where the first three values specify the
                      # origin, the second 3 values specify the first axis and the last 3 values
                      # specify the second axis. In this case we scan the ZX plane but the origin
                      # is not at the IP at y=-1 to create a symmetric cut view in |y|< 1 cm
                      # Origin
                      # First axis
                      # Second axis
                      # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
                      # will stop the ray after reaching the given distance from the start point
                      # Number of steps along the first axis
                      # Minimum value for the first axis
                      # Maximum value for the first axis
                      # Number of steps along the second axis
                      # Minimum value for the second axis
                      # Maximum value for the second axis
                      # Split output by Material names insted of by Region
                      # Specify the names of Materials to ignore in the scan.
                      # Default is Air and Vacuum
    'Filename': 'MaterialScan.root',
    'spherical': True,
    'spherical.origin': [0, 0, 0],
    'spherical.nTheta': 1000,
    'spherical.minTheta': 17,
    'spherical.maxTheta': 150,
    'spherical.cosTheta': True,
    'spherical.nPhi': 1000,
    'spherical.minPhi': 0,
    'spherical.maxPhi': 360,
    'spherical.maxDepth': 0,
    'spherical.splitByMaterials': False,
    'spherical.ignored': ['Air', 'Vacuum', 'G4_AIR', 'ColdAir'],
    'planar': True,
    'planar.plane': 'custom',
    'planar.custom': [
        0,
        -16,
        0,
        0,
        0,
        1,
        1,
        0,
        0,
    ],
    'planar.maxDepth': 32,
    'planar.nU': 2000,
    'planar.minU': -50.0,
    'planar.maxU': 60,
    'planar.nV': 2000,
    'planar.minV': -16,
    'planar.maxV': 16,
    'planar.splitByMaterials': True,
    'planar.ignored': ['Air', 'G4_AIR', 'ColdAir'],
})

# Create a path and add all modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(materialscan)

# Do the MaterialScan, can take some time depending on the number of steps
process(main)
