#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys

rootFileName = '../UDSTtoNTUP'
nOfEvents = 10
logFileName = rootFileName + '.log'

sys.stdout = open(logFileName, 'w')

import os
from basf2 import *

main = create_path()

# --------------------------------------------------------------
# set_log_level(LogLevel.DEBUG)
input = register_module('RootInput')
input.param('inputFileName', '../MDSTtoUDST.udst.root')
main.add_module(input)

# ---------------------------------------------------------------
# Show progress of processing
progress = register_module('Progress')
gearbox = register_module('Gearbox')
main.add_module(progress)
main.add_module(gearbox)

# ----------------------------------------------------------------
# Add the geometry for Rave
geometry = register_module('Geometry')
components = [
    'MagneticField',
    'BeamPipe',
    'PXD',
    'SVD',
    'CDC',
    'TOP',
    'ARICH',
    'ECL',
    'BKLM',
    'EKLM',
    'ESTR',
    'Coil',
    'STR',
    ]
geometry.param('Components', components)
main.add_module(geometry)

# ---------------------------------------------------------------
# Analysis module to check basic analysis input containers
# analysis = register_module('AnalysisValidation')
# output root file name (the suffix .root will be added automaticaly)
# analysis.param('outputFileName', rootFileName)
# specify the names of Track collections
# analysis.param('GFTrackCandidatesColName', 'GFTrackCands')
# analysis.param('GFTracksColName', 'GFTracks')
# analysis.param('TracksColName', 'Tracks')
# main.add_module(analysis)

# ---------------------------------------------------------------
# Produce a microdst with Particles and MCParticles only (event meta data is included by default)
process(main)

# Print call statistics
print statistics
