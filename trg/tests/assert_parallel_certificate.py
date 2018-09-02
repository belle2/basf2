# Test for the parallel certified flag in all modules in the standard L1 trigger simulation.

from basf2 import *
from L1trigger import add_tsim

# Create an empty path
path = create_path()

# Add only tsim
add_tsim(path)

# Assert that all modules have a parallel processing certified flag.
modules = path.modules()
for m in modules:
    assert m.has_properties(
        ModulePropFlags.PARALLELPROCESSINGCERTIFIED), '%s is missing c_ParallelProcessingCertified flag!' % (m)
