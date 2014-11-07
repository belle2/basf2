#!/usr/bin/env python
# -*- coding: utf-8 -*-

# copy a file by reading it using RootInput and writing it again
# both input and output file should be given as command line arguments, e.g.:
# basf2 framework/examples/copy_file.py -i in.root -o out.root

from basf2 import *

main = create_path()

input = register_module('RootInput')
input.param('branchNames', ['SVDSimHits', 'SVDTrueHits',
                            'SVDTrueHitsToSVDSimHits'])
# enter the path to the input data file(s) on your system here
input.param('inputFileName', '/data/belle2/BG/Jun2014/bg_full/Touschek_LER_100us.root')
main.add_module(input)

output = register_module('RootOutput')
output.param('outputFileName', 'Touschek_LER_100us_SVD.root')
main.add_module(output)


main.add_module(register_module('ProgressBar'))


process(main)

print statistics
