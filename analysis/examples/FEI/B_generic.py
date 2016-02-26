#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *

particles = get_default_channels()

path = create_path()
path.add_module('RootInput')
path.add_module('Gearbox')
path.add_module('Geometry', ignoreIfPresent=True, components=['MagneticField'])

feistate = fullEventInterpretation(None, path, particles)

print(feistate.path)
process(feistate.path)
