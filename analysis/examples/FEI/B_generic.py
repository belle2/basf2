#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FEI import *
from basf2 import *
from modularAnalysis import *


analysis_path = create_path()
analysis_path.add_module('RootOutput')

particles = get_default_channnels()
main = fullEventInterpretation(None, analysis_path, particles)

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
