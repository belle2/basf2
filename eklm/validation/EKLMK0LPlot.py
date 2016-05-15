#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>EKLMK0LOutput.root</input>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of EKLM K0L validation plots.</description>
</header>
"""

from basf2 import *
from EKLMK0LPlotModule import *

# Input.
root_input = register_module('RootInput')
root_input.param('inputFileName', '../EKLMK0LOutput.root')

# Plotting.
eklmk0lplot = EKLMK0LPlotModule()

# Create main path.
main = create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(eklmk0lplot)

# Run.
process(main)
