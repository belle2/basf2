#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')
geometry.param('ExcludedComponents', ['EKLM'])

input = register_module('RootInput')
input.param('inputFileName', '../EvtGenSimRec.root')

dedx = register_module('DedxPID')
dedx_params = {  # use default PDF file
    'useIndividualHits': True,
    'removeLowest': 0.15,
    'removeHighest': 0.15,
    'onlyPrimaryParticles': False,
    'usePXD': False,
    'useSVD': True,
    'useCDC': True,
    'trackDistanceThreshold': 4.0,
    'enableDebugOutput': True,
    'ignoreMissingParticles': False,
    }
dedx.param(dedx_params)

main = create_path()
main.add_module(input)

main.add_module(gearbox)
main.add_module(geometry)

main.add_module(dedx)

output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenSimRec_dedx.root')
# avoid some issues with missing dictionaries for rktrackrep etc.
output.param('branchNames', ['DedxLikelihoods', 'DedxTracks', 'EventMetaData'])
main.add_module(output)

process(main)
print statistics
