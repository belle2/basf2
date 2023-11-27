#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##########################################################################
# 100 e+ e- -> J/psi eta_c events are generated using PHOKHARA + EvtGen  #
# generator combination                                                  #
#                                                                        #
# This is an example of expert usage with setting and explanation of     #
# individual generator parameters. If you just need to generate using    #
# the default settings, please use                                       #
# generators.add_phokhara_evtgen_combination(). Example can be found in  #
# generators/validation/PhokharaEvtgenGenerate.py.                       #
##########################################################################

import basf2
import beamparameters
import generators

# Set the global log level.
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# Beam parameters.
beamparameters.add_beamparameters(main, "Y4S")

# PHOKHARA and EvtGen.
decay_file = basf2.find_file('generators/examples/PhokharaEvtgenDoubleCharmonium.dec')
generators.add_phokhara_evtgen_combination(
    main, ['J/psi', 'eta_c'], decay_file, beam_energy_spread=True)

# Output.
output = basf2.register_module('RootOutput')
output.param('outputFileName', 'phokhara_evtgen_double_charmonium.root')
main.add_module(output)

# Print MC particles.
main.add_module('PrintMCParticles', logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# Progress.
main.add_module('Progress')

# Generate events.
basf2.process(main)

# show call statistics
print(basf2.statistics)
