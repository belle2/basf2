#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# Example of using BeamBkgTagSetter module to set the background tag variable in SimHits
# and to store background sample equivalent time (realTime) into BackgroundMetaData
# Note: if BackgroundMetaData is found in the input file basf2 will exit with Error
#       (e.g. events have already been processed with this module)

set_log_level(LogLevel.WARNING)

# background (collision) files
indir = '~nakayama/basf2_opt/release_201312_8th/Work_MCgen/output1/'  # at KEKCC
outdir = 'beamBkg/'  # output directory

bkgType = 'Touschek_LER'
realTime = 10.0e3  # nano seconds
fileName = bkgType + '_10us.root'
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
    'ECLSimHits',
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
