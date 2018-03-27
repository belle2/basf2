#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

main = create_path()
main.add_module('SeqRootInput')
main.add_module('Progress')
output = register_module('RootOutput')
output.param('branchNames', [
    'EventMetaData',
    'RawFTSWs',
    'RawPXDs',
    'RawSVDs',
    'ROIs'
])
main.add_module(output)
process(main)
print(statistics)
