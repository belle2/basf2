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
    <output>KLMK0LOutput.root</output>
    <contact>Leo Piilonen (piilonen@vt.edu)</contact>
    <description>Generation of 2000 B -> J/psi K_L0 events for KLM validation.</description>
</header>
"""

import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction

basf2.set_log_level(basf2.LogLevel.WARNING)

# Fixed random seed
basf2.set_random_seed(123456)

# Create main path
main = basf2.create_path()

# Event data
eventinfosetter = basf2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [2000])

# Evtgen and beam parameters.
evtgen = basf2.register_module('EvtGenInput')
evtgen.param('userDECFile', basf2.find_file('klm/validation/btojpsikl0.dec'))

# Add progress bars
progress = basf2.register_module('Progress')
progressBar = basf2.register_module('ProgressBar')

# Output
output = basf2.register_module('RootOutput')
output.param('outputFileName', '../KLMK0LOutput.root')

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(evtgen)

add_simulation(path=main)
add_reconstruction(path=main)

main.add_module(progress)
main.add_module(progressBar)

main.add_module(output)

# Process the path
basf2.process(main)
