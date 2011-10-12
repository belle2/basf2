#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *
logging.log_level = LogLevel.WARNING

evtmetagen = register_module('EvtMetaGen')
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geant4 = register_module('FullSim')
matscan = register_module('MaterialScan')
matscan.set_log_level(LogLevel.INFO)

# Restrict Geometry to certain components, in this case only BeamPipe and PXD
geometry.param('Components', ['BeamPipe', 'PXD'])

# Create a detailed Materialbudget for the beampipe and the PXD
# 1) 1000x1000 raster starting from IP looking at the acceptance up to a
# distance of 100cm of the IP
# 2) 1000x1000 raster of the ZX plane between X=-3 to 3 cm and Z=-9 to 14cm,
# starting at Y=50µm and scanning the next 100µm along Y
matscan.param({
    'Filename': 'MaterialScan.root',
    'spherical': True,
    'spherical.origin': [0, 0, 0],
    'spherical.nTheta': 1000,
    'spherical.minTheta': 17,
    'spherical.maxTheta': 150,
    'spherical.nPhi': 1000,
    'spherical.minPhi': 0,
    'spherical.maxPhi': 360,
    'spherical.maxDepth': 100,
    'planar': True,
    'planar.plane': 'custom',
    'planar.nU': 1000,
    'planar.minU': -9.0,
    'planar.maxU': 14.0,
    'planar.nV': 1000,
    'planar.minV': -3,
    'planar.maxV': 3,
    'planar.maxDepth': 100e-4,
    'planar.custom': [
        0,
        -50e-4,
        0,
        0,
        0,
        1,
        1,
        0,
        0,
        ],
    })

evtmetagen.param('EvtNumList', [1])
evtmetagen.param('RunList', [1])

main = create_path()

main.add_module(evtmetagen)
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(geant4)
main.add_module(matscan)

process(main)
