#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test to ensure the reconstruction can be run standalone without load evtgen/simulation
performed before in the same process.
"""

import os
import tempfile
import basf2

evtgen_steering = basf2.find_file('reconstruction/tests/evtgen_no_mc.py_noexec')
reco_steering = basf2.find_file('reconstruction/tests/reco.py_noexec')

# create and move to temporary directory
with tempfile.TemporaryDirectory() as tempdir:
    print("Moving to temporary directory " + str(tempdir))
    os.chdir(tempdir)

    # run generator & simulation
    assert(0 == os.system("basf2 " + evtgen_steering))
    # run reconstruction only
    assert(0 == os.system("basf2 " + reco_steering + " -i evtgen_bbar_no_mc.root"))
    # Check if there is 1 event in the file
    assert(0 == os.system("b2file-check -n1 evtgen_bbar_no_mc.root"))
