#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#    script to simulate 10 charged muon tracks with             #
#    fixed transverse momentum using the ParticleGun            #
#                                                               #
#################################################################

"""
<header>
    <output>VTX_trackingEfficiency_pt_0.60GeV.root</output>
    <contact>software-tracking@belle2.org</contact>
    <description>Create events with 10 muon tracks with fixed pt value.</description>
</header>
"""

from basf2 import *
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction, get_generated_pt_value

set_random_seed(123457)

pt_value = get_generated_pt_value(4)

output_filename = '../VTX_trackingEfficiency_pt_%.2fGeV.root' % pt_value

print(output_filename)

path = create_path()

run_simulation(path, pt_value, useVTX=True)
run_reconstruction(path, output_filename, useVTX=True)

process(path)
