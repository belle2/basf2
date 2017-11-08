#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################
# 1 - generate
#
# Generate a sample of electron tracks for dE/dx calibration
# using the BabayagaNLO generator (ee(gamma) - large angle)
#
# Usage: basf2 1_generate.py
#
# Input: None
# Output: B2Electrons.root
#
# Contributors: Jake Bennett
#
# Example steering file - 2017 Belle II Collaboration
#############################################################

from basf2 import *
from simulation import add_simulation
from reconstruction import add_reconstruction, add_mdst_output

# main path
main = create_path()

# event info setter
main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=10000)

# add the BABAYAGA.NLO module
babayaganlo = register_module('BabayagaNLOInput')
babayaganlo.param('FinalState', 'ee')
babayaganlo.param('MaxAcollinearity', 180.0)
babayaganlo.param('ScatteringAngleRange', [15., 165.])
babayaganlo.param('FMax', 75000)
babayaganlo.param('MinEnergy', 0.01)
babayaganlo.param('Order', 'exp')
babayaganlo.param('DebugEnergySpread', 0.01)
babayaganlo.param('Epsilon', 0.00005)
main.add_module(babayaganlo)

# Preselection to reject events with either no photons or track
generatorpreselection = register_module('GeneratorPreselection')
generatorpreselection.param('nChargedMin', 0)
generatorpreselection.param('nChargedMax', 999)
generatorpreselection.param('MinChargedPt', 0.10)
generatorpreselection.param('MinChargedTheta', 0.)
generatorpreselection.param('MaxChargedTheta', 180.)
generatorpreselection.param('nPhotonMin', 1)
generatorpreselection.param('MinPhotonEnergy', 1.5)
generatorpreselection.param('MinPhotonTheta', 15.)
generatorpreselection.param('MaxPhotonTheta', 165.)
generatorpreselection.param('applyInCMS', True)
main.add_module(generatorpreselection)

# detector simulation and reconstruction
add_simulation(main)
add_reconstruction(main)

# Finally add mdst output
add_mdst_output(
    main,
    filename="B2Electrons.root",
    additionalBranches=[
        'TRGGDLResults',
        'KlIds',
        'KLMClustersToKlIds',
        'CDCDedxTracks'])

# add a progress bar
main.add_module('ProgressBar')

# generate events
process(main)

# show call statistics
print(statistics)
