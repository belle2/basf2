#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 10 charged muon tracks with             #
#    fixed transverse momentum using the ParticleGun            #
#                                                               #
#    written by Michael Ziegler, KIT                            #
#    michael.ziegler2@kit.edu                                   #
#                                                               #
#################################################################

"""
<header>
    <output>trackingEfficiency_pt_0.05GeV.root</output>
    <contact>software-tracking@belle2.org</contact>
    <description>Create events with 10 muon tracks with fixed pt value.</description>
</header>
"""

import basf2 as b2
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction, get_generated_pt_value

b2.set_random_seed(123)

pt_value = get_generated_pt_value(0)

output_filename = '../trackingEfficiency_pt_%.2fGeV.root' % pt_value

print(output_filename)

path = b2.create_path()

run_simulation(path, pt_value)
run_reconstruction(path, output_filename)

b2.process(path)
