#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2

# -------------------------------------------------------------------------------------------------------
# Script options
magnet_options = {'magnet': True, 'no-magnet': False}
cosmic_options = {'cosmics': True, 'beam': False}

if len(sys.argv) < 3 or str(sys.argv[1]) not in magnet_options or str(sys.argv[2]) not in cosmic_options:
    print('Usage: basf2 collect.py magnet|no-magnet beam|cosmics -i INPUT_FILE [...]')
    print('Always specify if magnetic field was ON or OFF and if the source is from cosmic rays or machine beam')
    sys.exit(1)
# -------------------------------------------------------------------------------------------------------

excludedComponents = [] if magnet_options[str(sys.argv[1])] else ['MagneticField']
# We do not use RecoTracks as calibration input directly except cosmics
calibration_recotracks = ['RecoTracks'] if cosmic_options[str(sys.argv[2])] else []

set_random_seed(100)

main = create_path()

main.add_module('RootInput')
main.add_module('HistoManager', histoFileName='CollectorOutput.root')
main.add_module('Gearbox')
main.add_module('Geometry', excludedComponents=excludedComponents, useDB=False)
main.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)

"""
Here you can specify which DB objects will be considered
in Millepede calibration. Setting it to [] will collect
data for all available objects/detectors.

Other option could be

>>> dbobjects = ['BeamParameters', 'CDCAlignment', 'CDCLayerAlignment']

the available objects are:

BeamParameters, VXDAlignment, CDCAlignment, CDCLayerAlignment,
CDCTimeWalks, CDCTimeZeros, BKLMALignment, EKLMALignment
"""
dbobjects = ['VXDAlignment', 'BeamParameters']

# All data sources from samples as possible input
main.add_module('MillepedeCollector',
                minPValue=0.,
                components=dbobjects,
                tracks=calibration_recotracks,
                particles=['mu+:bbmu'],
                vertices=[],
                primaryVertices=['Z0:mumu'],
                calibrateVertex=True,
                useGblTree=False)
main.add_module('Progress')
main.add_module('RootOutput')

process(main)
print(statistics)
