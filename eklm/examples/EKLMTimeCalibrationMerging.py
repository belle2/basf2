#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# EKLM time calibration.

from basf2 import *
import sys

root_input = register_module('RootInput')
root_input.param('inputFileNames', sys.argv[2:])
root_output = register_module('RootOutput')
root_output.param('outputFileName', sys.argv[1])

main = create_path()
main.add_module(root_input)
main.add_module(root_output)
process(main)
