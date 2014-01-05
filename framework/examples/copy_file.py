#!/usr/bin/env python
# -*- coding: utf-8 -*-

# copy a file by reading it using RootInput and writing it again
# both input and output file should be given as command line arguments, e.g.:
# basf2 framework/examples/copy_file.py -i in.root -o out.root

from basf2 import *

main = create_path()

input = register_module('RootInput')
main.add_module(input)

output = register_module('RootOutput')
main.add_module(output)

main.add_module(register_module('ProgressBar'))


process(main)

print statistics
