#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Test to ensure the reconstruction of VTX upgrade can be run standalone without load evtgen/simulation
performed before in the same process.
"""

import os
import tempfile
from ROOT import Belle2
import vtx

evtgen_steering = Belle2.FileSystem.findFile('reconstruction/tests/evtgen_upgrade.py_noexec')
reco_steering = Belle2.FileSystem.findFile('reconstruction/tests/reco_upgrade.py_noexec')

# create and move to temporary directory
with tempfile.TemporaryDirectory() as tempdir:
    print("Moving to temporary directory " + str(tempdir))
    os.chdir(tempdir)

    print("Setup GT for 5 layer vtx upgrade")
    os.environ['BELLE2_VTX_BACKGROUND_DIR'] = "None"
    os.environ['BELLE2_VTX_UPGRADE_GT'] = vtx.valid_upgrade_globaltags['upgrade_globaltag_vtx_5layer']

    # run generator & simulation
    assert (0 == os.system("basf2 " + evtgen_steering))
    # run reconstruction only
    assert (0 == os.system("basf2 " + reco_steering))
    # Check if there are 10 events in the file
    assert (0 == os.system("b2file-check -n10 evtgen_upgrade_bbar.root"))
