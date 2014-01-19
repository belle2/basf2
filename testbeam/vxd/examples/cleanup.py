#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

from basf2 import *

argvs = sys.argv
argc = len(argvs)

if argc != 3:
    print 'Run as:'
    print '    basf2 cleanup.py input.root output.root'
    print 'This steering file removes branches left over by the HLT that'
    print 'interfere with reconstruction.'
    exit(1)

inputFileName = argvs[1]
outputFileName = argvs[2]

# create the main path
main = create_path()

input = register_module('RootInput')
input.param('inputFileName', inputFileName)
main.add_module(input)

output = register_module('RootOutput')
output.param('excludeBranchNames', [
        'SVDClusters',
        'SVDDigits',
        'SVDClustersToSVDDigits',
        'TrackCands',
        'TrackFitResults',
        'Tracks',
        ])
output.param('outputFileName', outputFileName)

main.add_module(output)

process(main)
