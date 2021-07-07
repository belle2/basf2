#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
basf2.set_random_seed(501)


class CountEvents(basf2.Module):
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

main = basf2.Path()
main.add_module("EventInfoSetter", evtNumList=[input_events], expList=[0], runList=[0])
prescale_mod = main.add_module('Prescale', prescale=0.01)

success_path = basf2.Path()
success_count = success_path.add_module(CountEvents())
prescale_mod.if_true(success_path)

basf2.process(main)

print(basf2.statistics)

assert success_count.num_events == expected_success_events
