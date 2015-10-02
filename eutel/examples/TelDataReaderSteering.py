#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# Set the log level to show only warning, error and, fatal messages
# otherwise there's gonna be a segfault when python exits
set_log_level(LogLevel.INFO)

# data reader
dataReader = register_module('TelDataReader')
# use 4394, 313 or, 3
p_dataReader = {'inputFileName': '../vxdtb/data/run000005.raw',
                'maxNumEntries': 10000}
dataReader.param(p_dataReader)

# data writer
dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', 'temp.root')

main = create_path()

main.add_module(dataReader)
main.add_module(dataWriter)

process(main)
