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
from KLMK0LPlotModule import *

# Input.
root_input = register_module('RootInput')
root_input.param('inputFileName', '../EKLMK0LOutput.root')

# Plotting.
klmk0lplot = KLMK0LPlotModule('EKLMK0L.root', False, True)

# Create main path.
main = create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(klmk0lplot)

# Run.
process(main)
