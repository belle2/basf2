#!/usr/bin/env python
# -*- coding: utf-8 -*-

########################################################

# Example steering file - 2011 Belle II Collaboration
########################################################

import os
import random
from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.DEBUG)

# EventInfoSetter - generate event meta data
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [20])

# Root file output
output = register_module('RootOutput')
output.param('outputFileName', 'muForBKLM.root')

gearbox = register_module('Gearbox')
print 'Gearbox registered '
gearbox.param('backends', ['file:./geometry/data/', 'file:./XMLdata/', 'file:'
              ])
print 'backend set '
gearbox.param('fileName', 'Belle2_red.xml')
# gearbox.param('InputFileXML','Belle2.xml')

bklmPack = register_module('BKLMRawPacker')
bklmUnpack = register_module('BKLMUnpacker')
# Create main path
main = create_path()

# Add modules to main path
main.add_module(eventinfosetter)
main.add_module(gearbox)

main.add_module(bklmPack)
main.add_module(bklmUnpack)
main.add_module(output)

# Process 100 events
process(main)

