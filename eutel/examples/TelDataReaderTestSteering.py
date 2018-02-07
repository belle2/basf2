#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

set_log_level(LogLevel.INFO)

# data reader
dataReader = register_module('TelDataReader')
# use 4394 or 3 or (better 313)
p_dataReader = {'inputFileName': '/home/benjamin/TB_2014_VXD/run000005.raw',
                'maxNumEntries': 15000}
dataReader.param(p_dataReader)

# data reader test/validation
dataReaderTest = register_module('TelDataReaderTest')

# data writer
dataWriter = register_module('RootOutput')
dataWriter.param('outputFileName', 'temp.root')

main = create_path()

main.add_module(dataReader)
main.add_module(dataReaderTest)
main.add_module(dataWriter)

process(main)
