##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Test for the parallel certified flag in all modules in the standard simulation.

import basf2 as b2
from simulation import add_simulation

# Create an empty path
path = b2.create_path()

# Add only the simulation
add_simulation(path, bkgfiles="some_file", simulateT0jitter=True)

# Assert that all modules have a parallel processing certified flag, except for BGOverlayInput.
for m in path.modules():
    if m.name() == 'BGOverlayInput':
        continue
    assert m.has_properties(
        b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED), f'{m} is missing c_ParallelProcessingCertified flag!'
