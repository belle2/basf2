#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
from basf2 import *

seqRootInput = fw.register_module('SeqRootInput')
simpleOutput = fw.register_module('SimpleOutput')

seqRootInput.param('inputFileName', 'SeqRootOutput.root')

main = fw.create_path()

main.add_module(seqRootInput)
main.add_module(simpleOutput)

fw.process(main)
