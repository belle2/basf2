#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>KLMK0LOutput.root</input>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of KLM K0L validation plots.</description>
</header>
"""

import basf2 as b2
from KLMK0LPlotModule import KLMK0LPlotModule

# Input.
root_input = b2.register_module('RootInput')
root_input.param('inputFileName', '../KLMK0LOutput.root')

# Plotting.
klmk0lplot = KLMK0LPlotModule('KLMK0L.root', True, False)

# Create main path.
main = b2.create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(klmk0lplot)

# Run.
b2.process(main)
