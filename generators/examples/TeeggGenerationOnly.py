#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# 100 radiative Bhabha events are generated using the
# TEEGG generator
#
# Example steering file
########################################################

from basf2 import *

# Set the global log level
set_log_level(LogLevel.INFO)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

# Register the BABAYAGA.NLO module
teegg = register_module('TeeggInput')

# Set the mode for the boost of the generated particles: 0 = no boost, 1 = BELLE II, 2 = BELLE
teegg.param('BoostMode', 1)

# CMS energy [GeV]  (default: from gearbox), set BoostMode to 0 if you want to use this option
# teegg.param('CMSEnergy', 1.3)

# CONFIG
teegg.param('CONFIG', 'GAMMA')

# RADCOR and MAXWEIGHTS
teegg.param('RADCOR', 'SOFT')
teegg.param('WGHT1M', 1.001)  # SOFT
teegg.param('WGHTMX', 1.150)  # SOFT

# TEVETO
teegg.param('TEVETO', 10.0)

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

# extra outfile (contains the used weights, user should check against FMax value)
teegg.param('ExtraFile', './teegg-extra.root')

# geometry parameter database
gearbox = register_module('Gearbox')

# Register the Progress module and the Python histogram module
progress = register_module('Progress')

# output
output = register_module('RootOutput')
output.param('outputFileName', './teegg-outfile.root')

# Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(teegg)
main.add_module(output)
# uncomment the following line if you want event by event info
# main.add_module("PrintMCParticles", logLevel=LogLevel.DEBUG, onlyPrimaries=False)

# generate events
process(main)

# show call statistics
print statistics
