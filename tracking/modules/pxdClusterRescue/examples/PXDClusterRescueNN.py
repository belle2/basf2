#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# register modules
rootInputModule = register_module('RootInput')
filename_data = '<PATH TO SIMULATION DATA>/PXDClusterSimulationData.root'
rootInputModule.param({'inputFileName': filename_data})

gearboxModule = register_module('Gearbox')
geometryModule = register_module('Geometry')

PXDClusterRescueNN = register_module('PXDClusterRescueNN')
PXDClusterRescueNN.param({'filenameExpertise': '../data/PXDClusterNeuroBayes.nb'})
PXDClusterRescueNN.param({'classThreshold': 0.5})
PXDClusterRescueNN.param({'namePXDClusterInput': ''})
PXDClusterRescueNN.param({'namePXDClusterOutput': ''})

# create path and add modules
main = create_path()

main.add_module(rootInputModule)
main.add_module(gearboxModule)
main.add_module(geometryModule)

main.add_module(PXDClusterRescueNN)

# process
process(main)
