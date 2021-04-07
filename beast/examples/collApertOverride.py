#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###########################################################################
# This steering file demonstrates how to override collimator displacements,
# create the Belle II detector geometry and check for overlaps
###########################################################################

from basf2 import Path, process
from sys import argv

# Create main path
main = Path()
# Add modules to main path
main.add_module("EventInfoSetter")
# Geometry parameter loader
gearbox = main.add_module("Gearbox")
# Geometry root file
gearbox.param('fileName', '/geometry/Belle2.xml')
# Override collimator displacement
gearbox.param('override', [
    ("/DetectorComponent[@name='FarBeamLine']/Content/Collimator[@name='D01H5']/sec[@name='d1']", '-15', 'mm'),
    ("/DetectorComponent[@name='FarBeamLine']/Content/Collimator[@name='D01H5']/sec[@name='d2']", '12', 'mm'),
])
# Geometry builder
main.add_module("Geometry", useDB=False,
                components=['FarBeamLine'],
                # components=['BeamPipe','Cryostat','VXDService','HeavyMetalShield','FarBeamLine'],
                )

# Overlap checker
if len(argv) > 1:
    main.add_module("OverlapChecker", points=int(argv[1]))
else:
    main.add_module("OverlapChecker")
# Save overlaps to file to be able to view them with b2display
main.add_module("RootOutput", outputFileName="Overlaps.root")
# Process one event
process(main)
