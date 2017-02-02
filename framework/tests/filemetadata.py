#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *
from basf2_version import version

set_random_seed("something important")

testFile = tempfile.NamedTemporaryFile(prefix='b2filemetadata_')

# Register modules
eventinfosetter = register_module('EventInfoSetter', evtNumList=[9, 1], runList=[1, 15], expList=[7, 7])
output = register_module('RootOutput')

output.param('outputFileName', testFile.name)
output.param('updateFileCatalog', False)

# Create paths
main = create_path()

# Add modules to paths
main.add_module(eventinfosetter)
main.add_module(output)

# Process events
process(main)

# Check the file meta data (via DataStore)
from ROOT import Belle2
metadata = Belle2.PyStoreObj('FileMetaData', 1)

# print (metadata.getLfn()) #?
assert 10 == metadata.getNEvents()

assert 7 == metadata.getExperimentLow()
assert 1 == metadata.getRunLow()
assert 1 == metadata.getEventLow()
assert 7 == metadata.getExperimentHigh()
assert 15 == metadata.getRunHigh()
assert 1 == metadata.getEventHigh()  # highest evt. no in highest run
assert metadata.containsEvent(7, 1, 1)
assert metadata.containsEvent(7, 1, 9)
assert metadata.containsEvent(7, 15, 1)
assert metadata.containsEvent(7, 1, 10)  # does not exist, but would be plausible
assert metadata.containsEvent(7, 3, 6)  # same
assert not metadata.containsEvent(7, 0, 10)  # does not exist, but would be plausible
assert not metadata.containsEvent(7, 16, 6)  # same
assert not metadata.containsEvent(0, 15, 6)

assert 0 == metadata.getNParents()

# print (metadata.getDate())
# assert socket.gethostname() == metadata.getSite()
# print (metadata.getUser()) #different env variables, not checked
assert "something important" == metadata.getRandomSeed()

assert version == metadata.getRelease()
assert metadata.getSteering().startswith('#!/usr/bin/env python3')
assert metadata.getSteering().strip().endswith('dummystring')
assert 10 == metadata.getMcEvents()


# Check the file meta data (via file)
assert 0 == os.system('showmetadata ' + testFile.name)

os.system('touch Belle2FileCatalog.xml')
assert 0 == os.system('addmetadata --lfn /logical/file/name ' + testFile.name)

assert 0 == os.system('showmetadata ' + testFile.name)

# dummystring
