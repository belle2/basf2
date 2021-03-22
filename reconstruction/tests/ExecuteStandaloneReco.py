#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test to ensure the reconstruction can be run standalone without load evtgen/simulation
performed before in the same process.
"""

import os
import tempfile
import basf2

evtgen_steering = basf2.find_file('reconstruction/tests/evtgen.py_noexec')
reco_steering = basf2.find_file('reconstruction/tests/reco.py_noexec')

# create and move to temporary directory
with tempfile.TemporaryDirectory() as tempdir:
    print("Moving to temporary directory " + str(tempdir))
    os.chdir(tempdir)

    # run generator & simulation
    assert(0 == os.system("basf2 " + evtgen_steering))
    # run reconstruction only
    assert(0 == os.system("basf2 " + reco_steering))
    # Check if there are 10 events in the file
    assert(0 == os.system("b2file-check -n10 evtgen_bbar.root"))
