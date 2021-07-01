#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# The VariablesToExtraInfo module saves variables at some stage in the
# processing chain for recovery later. In this example it saves the invariant
# mass of D0 candidates, before- and after- running a vertex fit. You could
# also use this to save electron energy (or dielectron q2) before- and after-
# running Bremsstrahlung correction or similar.
#
# Thomas Keck and Sam Cunliffe
#
# For full documentation please refer to https://software.belle2.org
# Anything unclear? Ask questions at https://questions.belle2.org

import basf2
import modularAnalysis as ma  # a shorthand for the analysis tools namespace
from vertex import kFit

mypath = basf2.Path()  # create a new path

# add input data and ParticleLoader modules to the path
ma.inputMdstList('default', [basf2.find_file('analysis/tests/mdst.root')], path=mypath)
ma.fillParticleLists([('K-', 'kaonID > 0.2'), ('pi+', 'pionID > 0.2')], path=mypath)
ma.reconstructDecay('D0 -> K- pi+', '1.750 < M < 1.95', path=mypath)
ma.matchMCTruth('D0', path=mypath)

# this saves the "pre-vertex fit" mass in the extraInfo() of the D0 particle
mypath.add_module('VariablesToExtraInfo',
                  particleList='D0',
                  variables={'M': 'M_before_vertex_fit'})

# Now we do a vertex fit (this can change the mass).
# All candidates are kept.
kFit('D0', -1.0, path=mypath)

# now save the pre- and post- fit mass using VariablesToNtuple
mypath.add_module('VariablesToNtuple',
                  particleList='D0',
                  variables=['M', 'extraInfo(M_before_vertex_fit)'])

# It is important to understand that modules are loaded onto a basf2.Path by
# the modularAnalysis convenience functions (or by hand by you) and the order
# matters so you need to put VariablesToExtraInfo in the path *before* your
# processing which may alter variables.
#
# Here is an example of how to check this:
basf2.B2RESULT("Printing modules on the path")
for i, module in enumerate(mypath.modules()):
    basf2.B2RESULT(i, "   ", module.name())
# We use B2RESULT because it stands out in amongst the basf2 information.
# I could use python's native print()

# process the data
basf2.process(mypath)
print(basf2.statistics)
