#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = register_module('SeqRootInput')
else:
    root_input = register_module('RootInput')

main = create_path()
main.add_module(root_input)

unpacker = register_module('CDCTriggerUnpacker')
unpacker.logging.log_level = LogLevel.DEBUG
unpacker.logging.debug_level = 10
unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
unpacker.param('unpackTracker2D', True)
unpacker.param('headerSize', 2)

main.add_module(unpacker)
main.add_module('RootOutput', outputFileName='unpackedCDCTrigger.root')
process(main)
print(statistics)
