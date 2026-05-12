#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <output>PhokharaEvtgenData.root</output>
    <contact>Kirill Chilikin (K.A.Chilikin@inp.nsk.su)</contact>
    <description>Generation of 25000 e+ e- -> J/psi eta_c events.</description>
</header>
"""

import basf2
import generators
from ROOT import Belle2

# Logging level.
basf2.set_log_level(basf2.LogLevel.INFO)

# Random seed.
basf2.set_random_seed(12345)

# Create the main path.
main = basf2.create_path()

# Event information
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=25000)

# Add combination of PHOKHARA and EvtGen.
user_decay_file_name = 'generators/examples/PhokharaEvtgenDoubleCharmonium.dec'
user_decay_file = Belle2.FileSystem.findFile(user_decay_file_name)
generators.add_phokhara_evtgen_combination(
    main, ['J/psi', 'eta_c'], user_decay_file, beam_energy_spread=False)

# Output.
output = basf2.register_module('RootOutput')
output.param('outputFileName', 'PhokharaEvtgenData.root')
main.add_module(output)

# Progress.
main.add_module('Progress')

# Generate events.
basf2.process(main)
