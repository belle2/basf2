#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# 100 radiative Bhabha events are generated using the BHWIDE
# generator
#
# Example steering file
########################################################

from basf2 import *

## Set the global log level
set_log_level(LogLevel.INFO)

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [100])  # we want to process 100 events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

## Register the BHWIDE module
bhwide = register_module('BHWideInput')

# Set the logging level for the BHWIDE module to INFO in order to see the cross sections etc.
bhwide.set_log_level(LogLevel.INFO)

# Set the mode for the boost of the generated particles: 0 = no boost, 1 = BELLE II, 2 = BELLE
bhwide.param('BoostMode', 1)

# min/max angle in CMS, applied to positron, in degrees CMS
bhwide.param('ScatteringAngleRangePositron', [15., 165.])

# min/max angle in CMS, applied to electron, in degrees CMS
bhwide.param('ScatteringAngleRangeElectron', [15., 165.])

# maximum acollinearity angle between finale state electron and positron, in degrees CMS
bhwide.param('MaxAcollinearity', 180.0)

# CMS energy (only for boost mode == 0)
#bhwide.param('CMSEnergy', 10.580)

# Minimal energy of the electron and positron in GeV CMS
bhwide.param('MinEnergy', 0.150)

# Vacuum polarization switch: off, eidelman, bhlumi and burkhardt (default)
bhwide.param('VacuumPolarization', 'burkhardt')

# Maximum weight, default=3.0, for <0: automatic search
bhwide.param('WtMax', 3.0)

# geometry parameter database
gearbox = register_module('Gearbox')

## Register the Progress module and the Python histogram module
progress = register_module('Progress')

# output
output = register_module('RootOutput')
output.param('outputFileName', './bhwide_out.root')

## Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(bhwide)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics
