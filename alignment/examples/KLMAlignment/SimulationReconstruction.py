#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from simulation import add_simulation
from reconstruction import add_mc_reconstruction
import glob

main = create_path()

main.add_module('RootInput', inputFileName=sys.argv[1])
# main.add_module('Cosmics')
# geometry parameter database
gearbox = register_module('Gearbox')
main.add_module(gearbox)
# detector geometry
geometry = register_module('Geometry')
geometry.param({
    "excludedComponents": ["MagneticField"],
    "additionalComponents": ["MagneticField2d"],
})
main.add_module(geometry)
# detector simulation
g4sim = register_module('FullSim')
main.add_module(g4sim)
# digitization in simulator
pxd_digitizer = register_module('PXDDigitizer')
main.add_module(pxd_digitizer)
pxd_clusterizer = register_module('PXDClusterizer')
main.add_module(pxd_clusterizer)
svd_digitizer = register_module('SVDDigitizer')
main.add_module(svd_digitizer)
svd_clusterizer = register_module('SVDClusterizer')
main.add_module(svd_clusterizer)
cdc_digitizer = register_module('CDCDigitizer')
main.add_module(cdc_digitizer)
top_digitizer = register_module('TOPDigitizer')
main.add_module(top_digitizer)
arich_digitizer = register_module('ARICHDigitizer')
main.add_module(arich_digitizer)
ecl_digitizer = register_module('ECLDigitizer')
main.add_module(ecl_digitizer)
bklm_digitizer = register_module('BKLMDigitizer')
main.add_module(bklm_digitizer)
eklm_digitizer = register_module('EKLMDigitizer')
main.add_module(eklm_digitizer)

# reconstruction
add_mc_reconstruction(main)
"""
for module in main.modules():
  if module.name() == 'Ext':
    module.param('Cosmic', True)
"""
output = register_module('RootOutput')
output.param('outputFileName', sys.argv[2])
main.add_module(output)
main.add_module('Progress')
process(main)

# Print call statistics
print(statistics)
