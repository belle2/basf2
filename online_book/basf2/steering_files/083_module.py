#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2


class MinModule(b2.Module):
    """A minimal example of a basf2 module in python."""

    def __init__(self):
        """Constructor"""
        # call constructor of base class, required if you implement __init__
        # yourself!
        super().__init__()
        # and do whatever else is necessary like declaring member variables

    def initialize(self):
        """Called once in the beginning just before starting processing"""
        b2.B2INFO("initialize()")

    def beginRun(self):
        """Called every time a run changes before the actual events in that run
        are processed
        """
        b2.B2INFO("beginRun()")

    def event(self):
        """Called once for each event"""
        b2.B2INFO("event()")

    def endRun(self):
        """Called every time a run changes after the actual events in that run
        were processed
        """
        b2.B2INFO("endRun()")

    def terminate(self):
        """Called once after all the processing is complete"""
        b2.B2INFO("terminate()")


# create a path
main = b2.Path()

# set to generate 10 dummy events
main.add_module("EventInfoSetter", evtNumList=[10])

# and add our module
main.add_module(MinModule())

# run the path
b2.process(main)
