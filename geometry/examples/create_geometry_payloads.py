#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond no_doxygen

"""
Create a full set of consistent geometry payloads for nominal geometry, phase 2
geometry, and early phase 3 gometry from XML files.

Optionally one can give a list of payload names to keep only a subset of payloads
"""

import basf2
import b2test_utils
import shutil


# remove existing local database
shutil.rmtree("localdb", ignore_errors=True)

# create phase3 geometry: This is the default in Belle2.xml
phase3 = basf2.Path()
phase3.add_module("EventInfoSetter")
phase3.add_module("Gearbox")
phase3.add_module("Geometry", createPayloads=True, payloadIov=[0, 0, 0, -1])
b2test_utils.safe_process(phase3)

# create post LS1 geometry: Identical to the default one
postLS1 = basf2.Path()
postLS1.add_module("EventInfoSetter")
postLS1.add_module("Gearbox")
postLS1.add_module("Geometry", createPayloads=True, payloadIov=[1004, 0, 1004, -1])
b2test_utils.safe_process(postLS1)

# create phase2 geometry. For this we need to manually add all the phase2
# detectors and remove the phase3-only detectors.
phase2_detectors = "BeamPipe PXD SVD ServiceGapsMaterial MICROTPC PINDIODE BEAMABORT HE3TUBE CLAWS FANGS PLUME QCSMONITOR".split()
phase2 = basf2.create_path()
phase2.add_module("EventInfoSetter")
phase2.add_module("Gearbox")
phase2.add_module("Geometry", createPayloads=True, payloadIov=[1002, 0, 1002, -1],
                  excludedComponents=["BeamPipe", "PXD", "SVD", "VXDService", "ServiceGapsMaterial"],
                  additionalComponents=[e + "-phase2" for e in phase2_detectors])
b2test_utils.safe_process(phase2)

# create early phase3 geometry: same as phase 3 but different PXD configuration
early3 = basf2.Path()
early3.add_module("EventInfoSetter")
early3.add_module("Gearbox")
early3.add_module("Geometry", createPayloads=True, payloadIov=[1003, 0, 1003, -1],
                  excludedComponents=['PXD', "ServiceGapsMaterial"],
                  additionalComponents=['PXD-earlyPhase3', 'ServiceGapsMaterial-earlyPhase3'])
b2test_utils.safe_process(early3)

# @endcond
