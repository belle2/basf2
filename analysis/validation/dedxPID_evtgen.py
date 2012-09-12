#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import random
from basf2 import *

# create geometry
gearbox = register_module('Gearbox')
geometry = register_module('Geometry')

input = register_module('RootInput')
input.param('inputFileName', '../EvtGenSimRec.root')

dedx = register_module('DedxPID')
dedx_params = {
    'UseIndividualHits': True,
    'RemoveLowest': 0.15,
    'RemoveHighest': 0.15,
    'OnlyPrimaryParticles': False,
    'UsePXD': False,
    'UseSVD': True,
    'UseCDC': True,
    'TrackDistanceThreshold': 4.0,
    'EnableDebugOutput': True,
#use default PDF file
    'IgnoreMissingParticles': False,
    }
dedx.param(dedx_params)

main = create_path()
main.add_module(gearbox)
main.add_module(geometry)

main.add_module(input)

main.add_module(dedx)

output = register_module('RootOutput')
output.param('outputFileName', 'EvtGenSimRec_dedx.root')
#avoid some issues with missing dictionaries for rktrackrep etc.
output.param('branchNames', ['DedxLikelihoods', 'DedxTracks', 'EventMetaData'])
main.add_module(output)

process(main)
print statistics
