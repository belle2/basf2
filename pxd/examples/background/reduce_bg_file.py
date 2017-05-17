#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# copy a beam background simulatio file, keeping only data related to SVD.

from basf2 import *

main = create_path()

input = register_module('RootInput')
input.param('branchNames', ['PXDSimHits', 'PXDTrueHits',
                            'PXDTrueHitsToPXDSimHits', 'BeamBackHits'])
# enter the path to the input data file(s) on your system here
input.param('inputFileName', '/data/belle2/BG/Jun2014/bg_full/Touschek_LER_100us.root')
main.add_module(input)

bbfilter = register_module('PXDBeamBackHitFilter')
bbfilter.set_log_level(LogLevel.INFO)
main.add_module(bbfilter)

output = register_module('RootOutput')
# enter the path to the output file on your filesystem here
output.param('outputFileName', 'Touschek_LER_100us_PXD.root')
main.add_module(output)


main.add_module(register_module('ProgressBar'))


process(main)

print(statistics)
