#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# remove empty events from beam background data files.

import basf2 as b2
from ROOT import Belle2


class SVDTrigger(b2.Module):

    """Returns 1 if current event contains at least one SVDSimHit or one
       SVD-related BeamBackHit, 0 otherwise"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

    def event(self):
        """reimplementation of Module::event()."""

        self.return_value(0)
        simhits = Belle2.PyStoreArray('SVDSimHits')
        bghits = Belle2.PyStoreArray('BeamBackHits')
        if len(simhits) > 0 or len(bghits) > 0:
            self.return_value(1)


main = b2.create_path()

input = b2.register_module('RootInput')
main.add_module(input)

########################################
# only proceed to output with non-empty events.
trigger = SVDTrigger()
main.add_module(trigger)

# if SVDTrigger returns 0, we'll jump into an empty path
# (skipping output)
emptypath = b2.create_path()
trigger.if_false(emptypath)
########################################

output = b2.register_module('RootOutput')
main.add_module(output)

main.add_module(b2.register_module('ProgressBar'))

b2.process(main)

print(b2.statistics)
