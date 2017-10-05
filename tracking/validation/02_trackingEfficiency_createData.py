#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#################################################################
#                                                               #
#    script to simulate 10 charged tracks of configurable type  #
#    with fixed transverse momentum using the ParticleGun       #
#                                                               #
#    written by Michael Ziegler, KIT                            #
#    michael.ziegler2@kit.edu                                   #
#                                                               #
#################################################################

"""
<header>
    <output>trackingEfficiency_pt_0.05GeV.root</output>
    <contact>thomas.hauth@kit.edu</contact>
    <description>Create events with 10 tracks with fixed pt value.</description>
</header>
"""

from basf2 import *
from reconstruction import add_reconstruction, add_mc_reconstruction
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction
from tracking.validation.tracking_efficiency_helpers import get_generated_pt_value, additional_options

if len(sys.argv) != 3:
    sys.exit('Please provide PDG code of particle to be generated and index of pT values (0 to 9)!')

set_random_seed(10 * sys.argv[1] + sys.argv[2])

pt_value = get_generated_pt_value(int(sys.argv[2]))

output_filename = '../trackingEfficiency_pdg_' + sys.argv[1] + '_pt_%.2fGeV.root' % pt_value

print(output_filename)

path = create_path()

run_simulation(path, pt_value)

add_reconstruction(path, None, pruneTracks=0)
# add_mc_reconstruction(path, get_reconstruction_components(), pruneTracks=0)

tracking_efficiency = register_module('ECLTrackingPerformance')
# tracking_efficiency.logging.log_level = LogLevel.DEBUG
tracking_efficiency.param('outputFileName', output_file_name)
path.add_module(tracking_efficiency)

additional_options(path)

process(path)
