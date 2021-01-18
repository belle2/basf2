#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import Path, process
from sys import argv

main = Path()

main.add_module('RootInput')

masterclass = main.add_module('MasterClass')
if (len(argv) > 1):
    masterclass.param('outputFileName', argv[1])

process(main)
