#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>KLMK0LOutput.root</input>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of KLM K0L validation plots.</description>
</header>
"""

from basf2 import *
from KLMK0LPlotModule import *

# Input.
root_input = register_module('RootInput')
root_input.param('inputFileName', '../KLMK0LOutput.root')

# Plotting.
klmk0lplot = KLMK0LPlotModule('KLMK0L.root', True, False)

# Create main path.
main = create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(klmk0lplot)

# Run.
process(main)
