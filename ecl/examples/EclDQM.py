#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys
from subprocess import call
from basf2 import *

set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.DEBUG)

# Register necessary modules
input = register_module('SeqRootInput')

unpacker = register_module("ECLUnpacker")
unpacker.param("storeTrigTime", True)

calib = register_module('ECLDigitCalibrator')

histo = register_module('HistoManager')

ecldqm = register_module('ECLDQM')

# Create paths
main = create_path()
main.add_module(input)
main.add_module(unpacker)
main.add_module(calib)
main.add_module(histo)
main.add_module(ecldqm)

# Process all events
process(main)
print(statistics)
