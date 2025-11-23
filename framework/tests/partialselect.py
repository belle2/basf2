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


class CountEvents(basf2.Module):
    """Count the number of events passing this module"""

    def initialize(self):
        """Start number of events at zero"""
        #: Stores the total number of events passing this module
        self.num_events = 0
        #: Stores the list of events passing this module
        self.events = []

    def event(self):
        """Increment number of events each event"""
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        self.events.append(evtmetadata.obj().getEvent())
        self.num_events += 1


input_events = 1000
expected_success_events = 200
# range input for partialselect module
entry_start = 0.3
entry_stop = 0.5
# subsequent event range boundaries for the above inputs
expected_event_start = 301
expected_event_stop = 500

main = basf2.Path()
main.add_module("EventInfoSetter", evtNumList=[input_events], expList=[0], runList=[0])
partial_select_mod = main.add_module('PartialSelect', entryStart=entry_start, entryStop=entry_stop)

success_path = basf2.Path()
success_count = success_path.add_module(CountEvents())
partial_select_mod.if_true(success_path)

basf2.process(main)

print(basf2.statistics)

assert success_count.num_events == expected_success_events
assert success_count.events[0] == expected_event_start
assert success_count.events[-1] == expected_event_stop
