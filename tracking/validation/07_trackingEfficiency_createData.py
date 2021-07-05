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
    <output>trackingEfficiency_pt_0.60GeV.root</output>
    <contact>software-tracking@belle2.org</contact>
    <description>Create events with 10 muon tracks with fixed pt value.</description>
</header>
"""

import basf2 as b2
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction, get_generated_pt_value

b2.set_random_seed(123457)

pt_value = get_generated_pt_value(4)

output_filename = '../trackingEfficiency_pt_%.2fGeV.root' % pt_value

print(output_filename)

path = b2.create_path()

run_simulation(path, pt_value)
run_reconstruction(path, output_filename)

b2.process(path)
