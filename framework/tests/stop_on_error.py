#!/usr/bin/env python3

import os
import basf2

basf2.set_random_seed("")


class ErrorInInit(basf2.Module):
    """test"""

    def initialize(self):
        """reimplementation of Module::initialize()."""
        basf2.B2ERROR("In module " + self.name())


class ErrorInEvent(basf2.Module):
    """test"""

    def event(self):
        """reimplementation of Module::event()."""
        # error level to check that this doesn't prevent execution
        basf2.B2ERROR("In module " + self.name())


noerrors = basf2.Path()
noerrors.add_module('EventInfoSetter')

# no errors at all
basf2.process(noerrors)

errorsinevent = basf2.Path()
errorsinevent.add_module('EventInfoSetter')
errorsinevent.add_module(ErrorInEvent())

# no errors before event processing
basf2.process(errorsinevent)

# there were errors in event() of previous run
basf2.process(noerrors)
basf2.process(noerrors)
basf2.process(errorsinevent)
# there were some more errors in event() of previous run
basf2.process(noerrors)

# errors in initialize() -> fail
if os.fork() == 0:
    noerrors.add_module(ErrorInInit())
    basf2.process(noerrors)
else:
    assert os.wait()[1] != 0

# errors before process() -> fail
if os.fork() == 0:
    basf2.B2ERROR("htns")
    basf2.process(noerrors)
else:
    assert os.wait()[1] != 0

basf2.B2INFO('everything was OK.')
