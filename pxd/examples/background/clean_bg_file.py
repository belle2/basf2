#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# remove empty events from beam background data files.

import os
import random
from basf2 import *
from ROOT import Belle2


class SVDTrigger(Module):

    """Returns 1 if current event contains at least one PXDSimHit or one
       PXD-related BeamBackHit, 0 otherwise"""

    def initialize(self):
        """reimplementation of Module::initialize()."""

    def event(self):
        """reimplementation of Module::event()."""

        self.return_value(0)
        simhits = Belle2.PyStoreArray('PXDSimHits')
        bghits = Belle2.PyStoreArray('BeamBackHits')
        if len(simhits) > 0 or len(bghits) > 0:
            self.return_value(1)


main = create_path()

input = register_module('RootInput')
main.add_module(input)

########################################
# only proceed to output with non-empty events.
trigger = PXDTrigger()
main.add_module(trigger)

# if SVDTrigger returns 0, we'll jump into an empty path
# (skipping output)
emptypath = create_path()
trigger.if_false(emptypath)
########################################

output = register_module('RootOutput')
main.add_module(output)

main.add_module(register_module('ProgressBar'))

process(main)

print(statistics)
