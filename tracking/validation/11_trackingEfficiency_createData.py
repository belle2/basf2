#!/usr/bin/env python3

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
    <output>trackingEfficiency_pt_3.00GeV.root</output>
    <contact>software-tracking@belle2.org</contact>
    <description>Create events with 10 muon tracks with fixed pt value.</description>
</header>
"""

import basf2 as b2
from tracking.validation.tracking_efficiency_helpers import run_simulation, run_reconstruction, get_generated_pt_value

ACTIVE = True


def run():
    b2.set_random_seed(123460)

    pt_value = get_generated_pt_value(8)

    output_filename = f'../trackingEfficiency_pt_{pt_value:.2f}GeV.root'

    print(output_filename)

    path = b2.create_path()

    run_simulation(path, pt_value)
    run_reconstruction(path, output_filename)

    path.add_module('Progress')

    b2.process(path)

    print(b2.statistics)


if __name__ == '__main__':
    if ACTIVE:
        run()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
