#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *


class CountEvents(Module):

    """Count the number of events passing this module"""

    def initialize(self):
        self.num_events = 0

    def event(self):
        self.num_events += 1


input_events = 1000
main = create_path()
main.add_module("EventInfoSetter", evtNumList=[input_events], expList=[0], runList=[0])
prescale_mod = main.add_module('Prescale', prescale=0.01)

success_path = create_path()
success_count = success_path.add_module(CountEvents())
prescale_mod.if_true(success_path)

process(main)

print(statistics)

# This test MAY fail simply due to random chance. It should be incredibly rare and never happen, but the chance is there
assert success_count.num_events < input_events
