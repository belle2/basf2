#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

num_events = {}


class CountEvents(Module):

    """
    Count the number of events passing this module and create a dictionary
    of {(exp,run): events}
    """

    def initialize(self):
        pass

    def beginRun(self):
        global num_events
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        run = evtmetadata.obj().getRun()
        exp = evtmetadata.obj().getExperiment()
        if (exp, run) not in num_events:
            num_events[(exp, run)] = 0

    def event(self):
        global num_events
        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        event = evtmetadata.obj().getEvent()
        run = evtmetadata.obj().getRun()
        exp = evtmetadata.obj().getExperiment()
        num_events[(exp, run)] += 1


input_events = 1000
max_events = 500

main = create_path()
main.add_module("EventInfoSetter", evtNumList=[input_events] * 3, expList=[0, 0, 1], runList=[0, 1, 0])
eventlimiter_mod = main.add_module('EventLimiter', maxEventsPerRun=500)

success_path = create_path()
success_count = success_path.add_module(CountEvents())
eventlimiter_mod.if_true(success_path)

process(main)

print(statistics)

for passing_events in num_events.values():
    assert passing_events == max_events
