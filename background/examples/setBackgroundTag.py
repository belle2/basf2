#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# ---------------------------------------------------------------------------------------
# Example of preparing BG file to be used with BeamBkgMixer.
#
# BeamBkgTagSetter module is used to set the background tag variable in SimHits and
# to store background sample equivalent time (realTime) in BackgroundMetaData.
#
# This example uses pre-simulated BG sample (an ordinady root file with SimHits) as input.
# Alternatively, RootInput can be replaced with the full BG simulation.
#
# Note: if BackgroundMetaData already exists it will be overwritten!
#
# Two steering parameters of the module have to be specified:
#   - backgroundType: type of the background,
#                     valid types can be obtained by basf2 -m BeamBkgTagSetter
#   - realTime: time in nano seconds that corresponds to BG sample
# ---------------------------------------------------------------------------------------

set_log_level(LogLevel.WARNING)

# background (collision) file
indir = '~/nakayama/bgMC/release_201406_9thMCgen/output/'  # at KEKCC (but maybe empty now)
outdir = 'beamBkg/'  # output directory

bkgType = 'Touschek_LER'
realTime = 100.0e3  # nano seconds
fileName = bkgType + '_100us.root'
inFile = indir + fileName
outFile = outdir + fileName

# Create path
main = create_path()
emptyPath = create_path()

# Input
roinput = register_module('RootInput')
roinput.param('inputFileName', inFile)
main.add_module(roinput)

# set background tag in SimHits, leave main path if all SimHits empty
tagSetter = register_module('BeamBkgTagSetter')
tagSetter.param('backgroundType', bkgType)
tagSetter.param('realTime', realTime)
main.add_module(tagSetter)
emptyPath = create_path()
tagSetter.if_false(emptyPath)

# Output: SimHits only
output = register_module('RootOutput')
output.param('outputFileName', outFile)
output.param('branchNames', [
    'PXDSimHits',
    'SVDSimHits',
    'CDCSimHits',
    'TOPSimHits',
    'ARICHSimHits',
    'ECLHits',
    'BKLMSimHits',
    'EKLMSimHits',
    ])
main.add_module(output)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print statistics
