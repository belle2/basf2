#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.ERROR

# Create a all modules needed to set up simulation
evtmetagen = register_module('EvtMetaGen')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
simulation = register_module('FullSim')

# Create one Event in one Run
evtmetagen.param({
    'RunList': [1],
    'EvtNumList': [1],
})

# Restrict Geometry to certain components, in this case only PXD and SVD
geometry.param('Components', ["PXD", "SVD"])
geometry.set_log_level(LogLevel.INFO)

# Create the MaterialScan module
materialscan = register_module('MaterialScan')
materialscan.set_log_level(LogLevel.INFO)

# Create a detailed Materialbudget for the beampipe
# 1) 1000x1000 raster starting from IP looking at the acceptance
# 2) 2000x2000 raster of the ZX plane between X=-45 to 57.5 cm and Z=-16 to
#    16cm, starting at Y=-16cm and scanning the next 32cm along Y

materialscan.param({
    # Filename for output File
    'Filename': 'MaterialScan.root',

    # Do Spherical scan?
    'spherical': True,
    # Origin of the spherical scan
    'spherical.origin': [0, 0, 0],
    # Number of steps in theta
    'spherical.nTheta': 1000,
    # Minimal theta value
    'spherical.minTheta': 17,
    # Maximal theta value
    'spherical.maxTheta': 150,
    # Number of steps in theta
    'spherical.nPhi': 1000,
    # Minimal theta value
    'spherical.minPhi': 0,
    # Maximal theta value
    'spherical.maxPhi': 360,
    # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
    # will stop the ray after reaching the given distance from the start point
    'spherical.maxDepth': 0,
    # Split output by Material names instead of by Region
    'spherical.splitByMaterials': True,
    # Specify the names of Materials to ignore in the scan.
    # Default is Air and Vacuum
    #'spherical.ignore': ['Air','Vacuum','G4_AIR'],

    # Do Planar scan?
    'planar': True,
    # Name of the plane for scanning.  One of 'XY', 'XZ', 'YX', 'YZ', 'ZX',
    # 'ZY' or 'custom' to define the plane directly
    'planar.plane': 'custom',
    # Number of steps along the first axis
    'planar.nU': 2000,
    # Minimum value for the first axis
    'planar.minU': -45.0,
    # Maximum value for the first axis
    'planar.maxU': 57.5,
    # Number of steps along the second axis
    'planar.nV': 2000,
    # Minimum value for the second axis
    'planar.minV': -16,
    # Maximum value for the second axis
    'planar.maxV': 16,
    # Depth of the scan: 0 for scan the whole defined geometry. Any value >0
    # will stop the ray after reaching the given distance from the start point
    'planar.maxDepth': 32,
    # Define a custom plane for the scan, only used if planar.plane='custom'.
    # This is a list of 9 values where the first three values specify the
    # origin, the second 3 values specify the first axis and the last 3 values
    # specify the second axis.
    'planar.custom': [
        0, -16, 0,    # Origin
        0, 0, 1,      # First axis
        1, 0, 0,      # Second axis
    ],
    # Split output by Material names insted of by Region
    'planar.splitByMaterials': False,
    # Specify the names of Materials to ignore in the scan.
    # Default is Air and Vacuum
    #'planar.ignore': ['Air','Vacuum','G4_AIR'],
})

#Create a path and add all modules
main = create_path()
main.add_module(evtmetagen)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(materialscan)

#Do the MaterialScan, can take some time depending on the number of steps
process(main)
