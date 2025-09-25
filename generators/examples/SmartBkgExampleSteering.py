##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This example demonstrates a simple steering file using SmartBkg filtering to speed up
the production of feiHadronic skimmed charged B events.
"""

import basf2 as b2
import generators as gen
import simulation as sim
import reconstruction as rec
import modularAnalysis as ma
from skim.WGs.fei import feiHadronic

# For this example, load SmartBkg model and config from local database
b2.conditions.prepend_testing_payloads("localdb/database.txt")

# Initialize path and set event info (100 events for this example)
path = b2.Path()
path.add_module("EventInfoSetter", evtNumList=[100], expList=[0], runList=[0])

# Add event generator (evtgen for charged events in this example)
# Note that SmartBkg is only trained for charged, mixed, uubar, ddbar, ssbar, ccbar and taupair final states
finalstate = "charged"
gen.add_evtgen_generator(finalstate=finalstate, path=path)

# Define skim
skim = feiHadronic(
    analysisGlobaltag=ma.getAnalysisGlobaltag(),
    OutputFileName="test_smartbkg_fei.udst.root"
)

# Add SmartBkg filtering by providing the skim code and final state
gen.add_smartbkg_filtering(
    skim_code=skim.code,
    event_type=finalstate,
    path=path
)

# Add simulation and reconstruction
sim.add_simulation(path)
rec.add_reconstruction(path)

# Apply the skim
skim(path)

# Start event processing
b2.process(path)
