#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# suppress messages and warnings during processing:
set_log_level(LogLevel.ERROR)

# to run the framework the used modules need to be registered
eventinfosetter = register_module('EventInfoSetter')
# Setting the option for all non-hepevt reader modules:
eventinfosetter.param('evtNumList', [70])  # we want to process nr defined error events
eventinfosetter.param('runList', [1])  # from run number 1
eventinfosetter.param('expList', [1])  # and experiment number 1

packer = register_module('PXDPackerErr')
# [[dhhc1, dhh1, dhh2, dhh3, dhh4, dhh5] [ ... ]]
# -1 is disable port
packer.param('dhe_to_dhc', [
    [0, 2]
])
# packer.param('dhe_to_dhc', [
# [0,  2,  4, 34, 36, 38],
# [1,  6,  8, 40, 42, 44],
# [2, 10, 12, 46, 48, 50],
# [3, 14, 16, 52, 54, 56],
# [4,  3,  5, 35, 37, 39],
# [5,  7,  9, 41, 43, 45],
# [6, 11, 13, 47, 49, 51],
# [7, 15, 17, 53, 55, 57],
# ])

unpacker = register_module('PXDUnpacker')
unpacker.param('IgnoreMetaFlags', False)

packercheck = register_module('PXDPackerErr')
packercheck.param('dhe_to_dhc', [
    [0, 2]
])

logging.enable_summary(False)

# creating minimal path for test
main = create_path()
main.add_module(eventinfosetter)
main.add_module(packer)
unpacker.set_log_level(LogLevel.FATAL)  # this does not work yet, will fall back to ERROR
main.add_module(unpacker)
packercheck.set_log_level(LogLevel.INFO)  # tell us more in the log in case of any problem
main.add_module(packercheck, Check=True)

process(main)

# EOF.
