#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import tempfile
from basf2 import *
from ROOT import Belle2


assert 0 != os.system('basf2 --execute-path /this/path/doesnot/exist')

pathFile = tempfile.NamedTemporaryFile(prefix='b2pathtest_')


# uses Python modules, this should fail
steeringFile = Belle2.FileSystem.findFile('framework/tests/module_paths.py')
assert 0 != os.system('basf2 --dump-path ' + pathFile.name + ' ' + steeringFile)


# test actual execution
outputFile = tempfile.NamedTemporaryFile(prefix='b2pathtest_')

path = create_path()
path.add_module('EventInfoSetter', evtNumList=[2, 1], expList=[1, 1], runList=[1, 2])
path.add_module('RootOutput', outputFileName=outputFile.name)


# equivalent to --dump-path
fw.set_pickle_path(pathFile.name)
process(path)

# path dumped, but not executed
pathSize = os.stat(pathFile.name).st_size
assert 0 != pathSize
assert 0 == os.stat(outputFile.name).st_size

# equivalent to --execute-path
assert fw.get_pickle_path() == pathFile.name
process(None)

# path unmodified, output file created
assert pathSize == os.stat(pathFile.name).st_size
assert 0 != os.stat(outputFile.name).st_size
