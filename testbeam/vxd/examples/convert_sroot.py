#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

main = create_path()
main.add_module('SeqRootInput')
main.add_module('Progress')
main.add_module('RootOutput')
process(main)
print(statistics)
