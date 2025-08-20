##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This example demonstrates how to output model predictions and skim flags for
testing/validation of the SmartBkg filtering procedure.
"""

import basf2 as b2
import generators as gen
import simulation as sim
import reconstruction as rec
import modularAnalysis as ma
import mdst
from skim.WGs.fei import feiHadronic
from variables import variables as var

# For this example, load SmartBkg model and config from local database
b2.conditions.prepend_testing_payloads("localdb/database.txt")

# Initialize path and set event info (100 events for this example)
path = b2.Path()
path.add_module("EventInfoSetter", evtNumList=[100], expList=[0], runList=[0])

# Add event generator (evtgen for charged events in this example)
# Note that SmartBkg is only trained for charged, mixed, uubar, ddbar, ssbar, ccbar and taupair final states
finalstate = "charged"
gen.add_evtgen_generator(finalstate=finalstate, path=path)

# Define skim (all 51 skims defined as of August 2025 work with the standard SmartBkg model)
# Here we disable udst output because we only want the skim flags
skim = feiHadronic(
    analysisGlobaltag=ma.getAnalysisGlobaltag(),
    udstOutput=False
)

# Add SmartBkg filtering by providing the skim code and final state
# Here we enable debug mode so that no events are rejected or weighted and instead the model prediction is saved
# to the event extra info as 'SmartBKG_Prediction'
gen.add_smartbkg_filtering(
    skim_code=skim.code,
    event_type=finalstate,
    path=path,
    debug_mode=True
)

# Add simulation and reconstruction
sim.add_simulation(path)
rec.add_reconstruction(path)

# Optionally add mdst output so you can look at the events again
mdst.add_mdst_output(
    filename="test_events.mdst.root",
    path=path
)

# Apply the skim
skim(path)

# Write out skim flags and SmartBkg predictions as ntuple
var.addAlias("smartBkgPrediction", "eventExtraInfo(SmartBKG_Prediction)")
ma.variablesToNtuple(
    "",
    [skim.flag, "smartBkgPrediction"],
    treename="predictions",
    filename="test_flags_and_predictions.root",
    path=path
)

# Start event processing
b2.process(path)
