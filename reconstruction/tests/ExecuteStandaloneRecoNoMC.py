#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test to ensure the reconstruction can be run standalone without load evtgen/simulation
performed before in the same process.
"""

import os
import tempfile
import shutil
from ROOT import Belle2

evtgen_steering = Belle2.FileSystem.findFile('reconstruction/tests/evtgen_no_mc.py_noexec')
reco_steering = Belle2.FileSystem.findFile('reconstruction/tests/reco.py_noexec')

# create and move to temporary directory
tempdir = tempfile.mkdtemp()

try:
    print("Moving to temporary directory " + str(tempdir))
    os.chdir(tempdir)

    # run generator & simulation
    assert(0 == os.system("basf2 " + evtgen_steering))
    # run reconstruction only
    assert(0 == os.system("basf2 " + reco_steering + " -i evtgen_bbar_no_mc.root"))

finally:
    shutil.rmtree(tempdir)
