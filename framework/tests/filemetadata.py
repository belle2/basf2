#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# Register modules
eventinfosetter = register_module('EventInfoSetter')
output = register_module('RootOutput')

# Set module parameters
eventinfosetter.param('evtNumList', 10)

output.param('outputFileName', 'filemetadata.root')
output.param('updateFileCatalog', False)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(output)

# Process events
process(main)

# Check the file meta data
assert 0 == os.system('showmetadata filemetadata.root')

os.system('touch Belle2FileCatalog.xml')
assert 0 == os.system('addmetadata --id 12345 --guid ABCD --lfn /logical/file/name --logfile 67890 filemetadata.root')

assert 0 == os.system('showmetadata filemetadata.root')

os.remove('filemetadata.root')
