#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.ERROR

# Create a all modules needed to set up simulation
eventinfosetter = register_module('EventInfoSetter')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
simulation = register_module('FullSim')

# Create one Event in one Run
eventinfosetter.param({'runList': [1], 'evtNumList': [1]})

# Restrict Geometry to certain components, in this case only PXD and SVD
geometry.param('Components', ['BGO', 'MICROTPC'])
geometry.set_log_level(LogLevel.INFO)

# Create the MaterialScan module
materialscan = register_module('MaterialScan')
materialscan.set_log_level(LogLevel.INFO)

# Create a detailed Materialbudget for the beampipe
# 1) 1000x1000 raster starting from IP looking at the acceptance
# 2) 2000x2000 raster of the ZX plane between X=-45 to 57.5 cm and Z=-16 to
#    16cm, starting at Y=-16cm and scanning the next 32cm along Y

materialscan.param({  # Filename for output File
                      # Do Spherical scan?
                      # Origin of the spherical scan
                      # Number of steps in theta
                      # Minimal theta value
                      # Maximal theta value
                      # Number of steps in theta
                      # Minimal theta value
                      # Maximal theta value
                      # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
                      # will stop the ray after reaching the given distance from the start point
                      # Split output by Material names instead of by Region
                      # Specify the names of Materials to ignore in the scan.
                      # Default is Air and Vacuum
                      # 'spherical.ignore': ['Air','Vacuum','G4_AIR'],
                      # Do Planar scan?
                      # Name of the plane for scanning.  One of 'XY', 'XZ', 'YX', 'YZ', 'ZX',
                      # 'ZY' or 'custom' to define the plane directly
                      # Number of steps along the first axis
                      # Minimum value for the first axis
                      # Maximum value for the first axis
                      # Number of steps along the second axis
                      # Minimum value for the second axis
                      # Maximum value for the second axis
                      # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
                      # will stop the ray after reaching the given distance from the start point
                      # Define a custom plane for the scan, only used if planar.plane='custom'.
                      # This is a list of 9 values where the first three values specify the
                      # origin, the second 3 values specify the first axis and the last 3 values
                      # specify the second axis.
                      # Origin
                      # First axis
                      # Second axis
                      # Split output by Material names insted of by Region
    'Filename': 'MaterialScan.root',
    'spherical': True,
    'spherical.origin': [0, 0, 0],
    'spherical.nTheta': 1000,
    'spherical.minTheta': 17,
    'spherical.maxTheta': 150,
    'spherical.nPhi': 1000,
    'spherical.minPhi': 0,
    'spherical.maxPhi': 360,
    'spherical.maxDepth': 0,
    'spherical.splitByMaterials': True,
    'planar': True,
    'planar.plane': 'custom',
    'planar.nU': 2000,
    'planar.minU': -45.0,
    'planar.maxU': 57.5,
    'planar.nV': 2000,
    'planar.minV': -16,
    'planar.maxV': 16,
    'planar.maxDepth': 32,
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
    'planar.splitByMaterials': False,
    })
    # Specify the names of Materials to ignore in the scan.
    # Default is Air and Vacuum
    # 'planar.ignore': ['Air','Vacuum','G4_AIR'],

# Create a path and add all modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(materialscan)

# Do the MaterialScan, can take some time depending on the number of steps
process(main)
