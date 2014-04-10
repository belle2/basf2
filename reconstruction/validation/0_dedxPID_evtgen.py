#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

print os.getcwd()

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('excludedComponents', ['EKLM'])

input = register_module('RootInput')
input.param('inputFileName', '../EvtGenSimRec.root')
# original file already contains dE/dx info, but we want other options
input.param('excludeBranchNames', ['DedxLikelihoods', 'DedxTracks'])

dedx = register_module('DedxPID')
dedx_params = {  # use default PDF file
    'removeLowest': 0.15,
    'removeHighest': 0.15,
    'usePXD': False,
    'useSVD': True,
    'useCDC': True,
    'enableDebugOutput': True,
    }
dedx.param(dedx_params)

main = create_path()
main.add_module(input)

main.add_module(gearbox)
main.add_module(geometry)

main.add_module(dedx)

output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenSimRec_dedx.root')
# let's keep this small
output.param('branchNames', ['DedxLikelihoods', 'DedxTracks', 'EventMetaData'])
main.add_module(output)

process(main)
print statistics
