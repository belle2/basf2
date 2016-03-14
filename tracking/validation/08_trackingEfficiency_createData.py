#!/usr/bin/env python
# -*- coding: utf-8 -*-

# !/usr/bin/env python

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
    <output>trackingEfficiency_pt_1.00GeV.root</output>
    <contact>thomas.hauth@kit.edu</contact>
    <description>Create events with 10 muon tracks with fixed pt value.</description>
</header>
"""

from basf2 import *
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction, get_generated_pt_value

set_random_seed(123458)

pt_value = get_generated_pt_value(5)

output_filename = '../trackingEfficiency_pt_%.2fGeV.root' % pt_value

print output_filename

path = create_path()

run_simulation(path, pt_value)
run_reconstruction(path, output_filename)

process(path)
