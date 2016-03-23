#!/usr/bin/env python3
# -*- coding: utf-8 -*-

###############################################################################
# copy a file by reading it using RootInput and writing it again
# both input and output file should be given as command line arguments, e.g.:
# basf2 framework/examples/copy_file.py -i in.root -o out.root
###############################################################################

from basf2 import *

main = create_path()

# read files, needs -i param
main.add_module('RootInput')

# write file, you can set the file name via -o param
main.add_module('RootOutput')

# Show progress and remaining time
main.add_module('ProgressBar')


process(main)

print(statistics)
