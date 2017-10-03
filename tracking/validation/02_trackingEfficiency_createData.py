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
    <contact>thomas.hauth@kit.edu</contact>
    <description>Create events with 10 muon tracks with fixed pt value.</description>
</header>
"""

from basf2 import *
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction, get_generated_pt_value

if len(sys.argv) != 3:
    sys.exit('Please provide PDG code of particle to be generated and index of pT values (0 to 9)!')

set_random_seed(10 * sys.argv[1] + sys.argv[2])

pt_value = get_generated_pt_value(int(sys.argv[2]))

output_filename = '../trackingEfficiency_pdg_' + sys.argv[1] + '_pt_%.2fGeV.root' % pt_value

print(output_filename)

path = create_path()

run_simulation(path, pt_value)
run_reconstruction(path, output_filename)

process(path)
