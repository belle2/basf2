#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

input = fw.register_module('HLTInput')
output = fw.register_module('SimpleOutput')

input.param('port', 20000)

main = fw.create_path()
main.add_module(input)
main.add_module(output)

# fw.process (main, 2)
fw.process(main, 1)
