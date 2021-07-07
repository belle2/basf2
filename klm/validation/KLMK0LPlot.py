#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <input>KLMK0LOutput.root</input>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of KLM K0L validation plots.</description>
</header>
"""

import basf2
from KLMK0LPlotModule import KLMK0LPlotModule

# Input.
root_input = basf2.register_module('RootInput')
root_input.param('inputFileName', '../KLMK0LOutput.root')

# Plotting.
klmk0lplot = KLMK0LPlotModule('KLMK0L.root', True, False)

# Create main path.
main = basf2.create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(klmk0lplot)

# Run.
basf2.process(main)
