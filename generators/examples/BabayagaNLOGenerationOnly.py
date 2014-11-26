#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################
# 100 radiative Bhabha events are generated using the BabayagaNLO
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

## Register the BABAYAGA.NLO module
babayaganlo = register_module('BabayagaNLOInput')

# Set the mode for the boost of the generated particles: 0 = no boost, 1 = BELLE II, 2 = BELLE
babayaganlo.param('BoostMode', 1)

# Set the logging level for the BABAYAGA.NLO module to INFO in order to see the cross sections etc.
babayaganlo.set_log_level(LogLevel.INFO)

# Events (weighted) to be used for maximum weight search before generation (recommended for ee, theta>15deg: use more than >500,000 events)
babayaganlo.param('SearchMax', 500000)

# Set rejection weight (speeds up generation by avoiding initial weight search)
# EXPERTS ONLY
#babayaganlo.param('FMax', 1e5)

# final state: ee or gg, mm is unstable!
babayaganlo.param('FinalState', 'ee')

# soft photon cutoff, final result is indepedent of the cut off as long as its small (<1e-3)
# photon multiplicity (and exclusive cross sections depent on that parameter)
# EXPERTS ONLY
#babayaganlo.param('Epsilon', 5.e-4)

# CMS energy [GeV]  (default: 10.58GeV)
babayaganlo.param('CMSEnergy', 10.580)

# minimum energy for leptons/photons in the final state, in GeV (default 0.01GeV)
babayaganlo.param('MinEnergyFrac', 0.01)

# maximum acollinearity angle between finale state leptons/photons in degrees
babayaganlo.param('MaxAcollinearity', 180.0)

# min/max angle in CMS (checked for both leptons and the highest energetic gammas)
babayaganlo.param('ScatteringAngleRange', [15., 165.])

# extra outfile (contains the used weights, check against FMax value)
babayaganlo.param('ExtraFile', 'extrababayaganlo.root')

# geometry parameter database
gearbox = register_module('Gearbox')

## Register the Progress module and the Python histogram module
progress = register_module('Progress')

# output
output = register_module('RootOutput')
output.param('outputFileName', './babayaganlo_out.root')

## Create the main path and add the modules
main = create_path()
main.add_module(eventinfosetter)
main.add_module(progress)
main.add_module(gearbox)
main.add_module(babayaganlo)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics
