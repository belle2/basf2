#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *

set_random_seed("something important")

testFile = tempfile.NamedTemporaryFile(prefix='b2filemetadata_')

# Register modules
eventinfosetter = register_module('EventInfoSetter')
output = register_module('RootOutput')

# Set module parameters
eventinfosetter.param('evtNumList', 10)

output.param('outputFileName', testFile.name)
output.param('updateFileCatalog', False)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(output)

# Process events
process(main)

# Check the file meta data
assert 0 == os.system('showmetadata ' + testFile.name)

os.system('touch Belle2FileCatalog.xml')
assert 0 == os.system('addmetadata --lfn /logical/file/name ' + testFile.name)

assert 0 == os.system('showmetadata ' + testFile.name)
