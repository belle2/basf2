#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Generation of Belle MC.

import basf2
from generators import add_evtgen_generator

# Use B2BII local cache
basf2.conditions.metadata_providers = ["/sw/belle/b2bii/database/conditions/b2bii.sqlite"]
basf2.conditions.payload_locations = ["/sw/belle/b2bii/database/conditions/"]

# Use B2BII global tag.
basf2.conditions.override_globaltags()
basf2.conditions.prepend_globaltag('b2bii_beamParameters_with_smearing')

# Path.
main = basf2.create_path()

# Generate for experiment 55, run 0 (run-independent MC).
main.add_module('EventInfoSetter', expList=55, runList=0, evtNumList=100)

# Add generator.
add_evtgen_generator(path=main, finalstate='charged')

# Add output.
main.add_module('BelleMCOutput', outputFileName='charged.dat')

# Progress.
main.add_module('Progress')

# Generate events.
basf2.process(main)

# Statistics.
print(basf2.statistics)
