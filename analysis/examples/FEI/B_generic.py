#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from fei import *
from basf2 import *
from modularAnalysis import *

particles = get_default_channnels()
feistate = fullEventInterpretation(None, particles)
feistate.path.add_module('RootOutput')

# show constructed path
print feistate.path

process(feistate.path)

B2WARNING('event() statistics:')
print statistics
