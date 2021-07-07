#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from ROOT import Belle2

#: Dictionary of the number of events passing this module in each (Exp,Run)
num_events = {}


class CountEvents(basf2.Module):

    """
    Count the number of events passing this module and create a dictionary
    of {(exp,run): events}
    """

    def initialize(self):
        """Not implemented"""

    def beginRun(self):
        """Creates an entry in the global num_events dictionary"""
        global num_events
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        run = evtmetadata.obj().getRun()
        exp = evtmetadata.obj().getExperiment()
        if (exp, run) not in num_events:
            num_events[(exp, run)] = 0

    def event(self):
        """Increments the right (Exp,Run) entry in the num_events dictionary"""
        global num_events
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        run = evtmetadata.obj().getRun()
        exp = evtmetadata.obj().getExperiment()
        num_events[(exp, run)] += 1


input_events = 1000
max_events = 500

main = basf2.Path()
main.add_module("EventInfoSetter", evtNumList=[input_events] * 3, expList=[0, 0, 1], runList=[0, 1, 0])
eventlimiter_mod = main.add_module('EventLimiter', maxEventsPerRun=500)

success_path = basf2.Path()
success_count = success_path.add_module(CountEvents())
eventlimiter_mod.if_true(success_path)

basf2.process(main)

print(basf2.statistics)

for passing_events in num_events.values():
    assert passing_events == max_events
