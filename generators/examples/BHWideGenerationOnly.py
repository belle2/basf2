#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# 100 radiative Bhabha events are generated using the BHWIDE
# generator and the Beam Parameter Class
#
# Example steering file
########################################################

import basf2
# Set the global log level
basf2.set_log_level(basf2.LogLevel.INFO)

main = basf2.create_path()

# event info setter
main.add_module("EventInfoSetter", expList=0, runList=1, evtNumList=100)

# Register the BHWIDE module
bhwide = basf2.register_module('BHWideInput')
bhwide.set_log_level(basf2.LogLevel.INFO)

# min/max angle in CMS, applied to positron, in degrees CMS
bhwide.param('ScatteringAngleRangePositron', [15., 165.])

# min/max angle in CMS, applied to electron, in degrees CMS
bhwide.param('ScatteringAngleRangeElectron', [15., 165.])

# maximum acollinearity angle between finale state electron and positron, in degrees CMS
bhwide.param('MaxAcollinearity', 180.0)

# Minimal energy of the electron and positron in GeV CMS
bhwide.param('MinEnergy', 0.10)

# Vacuum polarization switch: off, eidelman, bhlumi and burkhardt (default)
bhwide.param('VacuumPolarization', 'burkhardt')

# EW corrections on (default) or off
bhwide.param('WeakCorrections', True)

# Maximum weight, default=3.0, for <0: automatic search
bhwide.param('WtMax', 3.0)

# gearbox
gearbox = basf2.register_module('Gearbox')

# Create the main path and add the modules
main.add_module("Progress")
main.add_module(gearbox)
main.add_module(bhwide)
main.add_module("RootOutput", outputFileName="bhwide.root")

main.add_module("PrintMCParticles", logLevel=basf2.LogLevel.DEBUG, onlyPrimaries=False)

# generate events
basf2.process(main)

# show call statistics
print(basf2.statistics)
