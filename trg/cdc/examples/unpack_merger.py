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

merger_unpacker = register_module('CDCTriggerUnpacker')
merger_unpacker.logging.log_level = LogLevel.DEBUG
merger_unpacker.logging.debug_level = 100
merger_unpacker.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)

main.add_module(merger_unpacker)
process(main)
print(statistics)
