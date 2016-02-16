#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *
from beamparameters import add_beamparameters

particles = get_default_channels()

path = create_path()
path.add_module('RootInput')
path.add_module('Gearbox')
path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])
beamparameters = add_beamparameters(path, 'Y4S')
feistate = fullEventInterpretation(None, path, particles)

if feistate.is_trained:
    open('FEI_finished_its_training', 'a').close()

# show constructed path
print(feistate.path)

process(feistate.path)

B2WARNING('event() statistics:')
print(statistics)
