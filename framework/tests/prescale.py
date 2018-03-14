#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
set_random_seed(501)


class CountEvents(Module):
    """Count the number of events passing this module"""

    def initialize(self):
        """Start number of events at zero"""
        #: Stores the total number of events passing this module
        self.num_events = 0

    def event(self):
        """Increment number of events each event"""
        self.num_events += 1


input_events = 1000
expected_success_events = 6

main = create_path()
main.add_module("EventInfoSetter", evtNumList=[input_events], expList=[0], runList=[0])
prescale_mod = main.add_module('Prescale', prescale=0.01)

success_path = create_path()
success_count = success_path.add_module(CountEvents())
prescale_mod.if_true(success_path)

process(main)

print(statistics)

assert success_count.num_events == expected_success_events
