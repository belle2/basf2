#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# Create module
test = fw.register_module('Hello')

# Create path
main = fw.create_path()

# Add module to path
main.add_module(test)

# Start event processing
fw.process(main, 100)
