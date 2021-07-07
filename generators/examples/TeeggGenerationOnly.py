#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import set_log_level, LogLevel, register_module, create_path, process, statistics

# Set the global log level
set_log_level(LogLevel.INFO)

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# Register the BABAYAGA.NLO module
teegg = register_module('TeeggInput')

# WEIGHTED (0) or UNWEIGHTED (1)
teegg.param('UNWEIGHTED', 1)

# CONFIG
teegg.param('CONFIG', 'GAMMA')

# RADCOR and MAXWEIGHTS
teegg.param('RADCOR', 'HARD')
teegg.param('WGHT1M', 1.001)  # SOFT
teegg.param('WGHTMX', 1.150)  # SOFT

# TEVETO
teegg.param('TEVETO', 5.0)
teegg.param('EEVETO', 0.5)

# TGMIN
teegg.param('TGMIN', 12.5)

# TEMIN
# teegg.param('TEMIN', 0.26180* 180.0 / 3.1415)

# CUTOFF
teegg.param('CUTOFF', 0.0070)

# PEGMIN
# teegg.param('PEGMIN', 0.78540* 180.0 / 3.1415)

# EEMIN
# teegg.param('EEMIN', 5.0)

# EGMIN
teegg.param('EGMIN', 0.50)

# UNWGHT
# teegg.param('UNWGHT', 1)

# output
output = register_module('RootOutput')
output.param('outputFileName', './teegg-outfile.root')

# Create the main path and add the modules
main.add_module("Progress")
main.add_module(teegg)
main.add_module(output)
# uncomment the following line if you want event by event info
main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print(statistics)
