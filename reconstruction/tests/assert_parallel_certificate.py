##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Test for the parallel certified flag in all modules in the standard reconstruction.
# It creates a path and fills it with all reconstruction modules, without anything else
# (this path can never be executed, but we do not want that anyway).
# Then it goes through all modules and checks it flag.

import basf2
import reconstruction

# Create an empty path
path = basf2.create_path()

# Add only the reconstruction
reconstruction.add_reconstruction(path)

# Assert that all modules have a parallel processing certified flag.
modules = path.modules()
for m in modules:
    assert m.has_properties(
        basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED), '%s is missing c_ParallelProcessingCertified flag!' % (m)
