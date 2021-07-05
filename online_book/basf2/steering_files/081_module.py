#!/usr/bin/env python3

import basf2 as b2


class MinModule(b2.Module):
    """Very minimal class to print Hello World in each event"""

    def event(self):
        """Event function, called once for each event"""
        b2.B2INFO("Hello World!")


# create a path
main = b2.Path()

# set to generate 10 dummy events
main.add_module("EventInfoSetter", evtNumList=[10])

# and add our module
main.add_module(MinModule())

# run the path
b2.process(main)
