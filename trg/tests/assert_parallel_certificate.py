# Test for the parallel certified flag in all modules in the standard L1 trigger simulation.

import basf2 as b2
from L1trigger import add_tsim

# Create an empty path
path = b2.create_path()

# Add only tsim
add_tsim(path)

# Assert that all modules have a parallel processing certified flag.
modules = path.modules()
for m in modules:
    assert m.has_properties(
        b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED), '%s is missing c_ParallelProcessingCertified flag!' % (m)
