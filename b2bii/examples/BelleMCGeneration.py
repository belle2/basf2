#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Generation of Belle MC.

import basf2
from generators import add_evtgen_generator

# Use B2BII global tag.
basf2.conditions.prepend_globaltag('B2BII_MC')

# Path.
main = basf2.create_path()

# Generate for experiment 55, run 0 (run-independent MC).
main.add_module('EventInfoSetter', expList=55, runList=0, evtNumList=100)

# Override generation flags (allow vertex smearing).
main.add_module('OverrideGenerationFlags')

# Add generator.
add_evtgen_generator(path=main, finalstate='charged')

# Add output.
main.add_module('BelleMCOutput', outputFileName='charged.dat')

# Progress.
main.add_module('Progress')

# Generate events.
basf2.process(main)

# Statistics.
print(basf2.statistics)
