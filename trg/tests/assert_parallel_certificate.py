##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Test for the parallel certified flag in all modules in the standard L1 trigger simulation.

import basf2 as b2
from L1trigger import add_trigger_simulation

# Create an empty path
path = b2.create_path()

# Add only L1 trigger simulation
add_trigger_simulation(path)

# Assert that all modules have a parallel processing certified flag.
modules = path.modules()
for m in modules:
    assert m.has_properties(
        b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED), '%s is missing c_ParallelProcessingCertified flag!' % (m)
