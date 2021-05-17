#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
from ROOT import Belle2


input_files = Belle2.Environment.Instance().getInputFilesOverride()
if not input_files.empty() and input_files.front().endswith(".sroot"):
    root_input = b2.register_module('SeqRootInput')
else:
    root_input = b2.register_module('RootInput')

main = b2.create_path()
main.add_module(root_input)

merger_unpacker = b2.register_module('CDCTriggerUnpacker')
merger_unpacker.logging.log_level = b2.LogLevel.DEBUG
merger_unpacker.logging.debug_level = 100
merger_unpacker.logging.set_info(b2.LogLevel.DEBUG, b2.LogInfo.LEVEL | b2.LogInfo.MESSAGE)

main.add_module(merger_unpacker)
b2.process(main)
print(b2.statistics)
