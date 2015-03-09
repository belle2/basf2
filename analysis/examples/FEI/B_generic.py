#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *


analysis_path = create_path()
analysis_path.add_module('RootOutput')

particles = get_default_channnels()
feistate = fullEventInterpretation(None, analysis_path, particles)

# show constructed path
print feistate.path

process(feistate.path)
B2WARNING('event() statistics:')
print statistics
