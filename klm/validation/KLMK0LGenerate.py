#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <output>KLMK0LOutput.root</output>
    <contact>Leo Piilonen (piilonen@vt.edu)</contact>
    <description>Generation of 2000 B -> J/psi K_L0 events for KLM validation.</description>
</header>
"""

import basf2
from simulation import add_simulation
from reconstruction import add_reconstruction
from validationgenerators import add_evtgen_for_validation

# Fixed random seed
basf2.set_random_seed(123456)

# Create main path
main = basf2.create_path()

# Add modules to main path
main.add_module('EventInfoSetter', evtNumList=[2000])

add_evtgen_for_validation(main)
basf2.set_module_parameters(main, name='EvtGenInput', userDECFile=basf2.find_file('klm/validation/btojpsikl0.dec'))

add_simulation(path=main)
add_reconstruction(path=main)

main.add_module('Progress')
main.add_module('ProgressBar')

main.add_module('RootOutput', outputFileName='../KLMK0LOutput.root')

# Process the path
basf2.process(main)
