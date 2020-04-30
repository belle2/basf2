#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import tempfile
from ROOT import Belle2

testScript = Belle2.FileSystem.findFile('pxd/tests/streamer_test.py_noexec')

# create and move to temporary directory
with tempfile.TemporaryDirectory() as tempdir:
    print("Moving to temporary directory " + str(tempdir))
    os.chdir(tempdir)

    # might not be the most elegant way but cannot directly redirect ROOT Error
    os.system("basf2 " + testScript + " > output.log 2>&1")
    # and check if an Error related to TStreamer was thrown
    assert(0 == open('output.log', 'r').read().count('Error in <TStreamerInfo::Build>'))
