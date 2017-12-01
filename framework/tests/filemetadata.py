#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
import subprocess
import json
from basf2 import *
from basf2_version import version

set_random_seed("something important")

testFile = tempfile.NamedTemporaryFile(prefix='b2filemetadata_')

# Generate a small test file
main = create_path()
main.add_module('EventInfoSetter', evtNumList=[9, 1], runList=[1, 15], expList=[7, 7])
main.add_module('RootOutput', outputFileName=testFile.name, updateFileCatalog=False)
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

# Check JSON output (contains steering file, so we cannotuse .out)
metadata_output = subprocess.check_output(['showmetadata', '--json', testFile.name])
m = json.loads(metadata_output.decode('utf-8'))
assert 7 == m['experimentLow']
assert 1 == m['runLow']
assert 1 == m['eventLow']
assert 7 == m['experimentHigh']
assert 15 == m['runHigh']
assert 1 == m['eventHigh']
assert 'something important' == m['randomSeed']
assert 10 == m['nEvents']
assert isinstance(m['nEvents'], int)
assert '/logical/file/name' == m['LFN']

# steering file is in metadata, so we check for existence of this string:
# dummystring
