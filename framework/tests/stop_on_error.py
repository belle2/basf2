#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from ROOT import Belle2

set_random_seed("")


class ErrorInInit(Module):
    """test"""

    def initialize(self):
        """reimplementation of Module::initialize()."""
        B2ERROR("In module " + self.name())


class ErrorInEvent(Module):
    """test"""

    def event(self):
        """reimplementation of Module::event()."""
        # error level to check that this doesn't prevent execution
        B2ERROR("In module " + self.name())


noerrors = create_path()
noerrors.add_module('EventInfoSetter')

# no errors at all
process(noerrors)

errorsinevent = create_path()
errorsinevent.add_module('EventInfoSetter')
errorsinevent.add_module(ErrorInEvent())

# no errors before event processing
process(errorsinevent)

# there were errors in event() of previous run
process(noerrors)
process(noerrors)
process(errorsinevent)
# there were some more errors in event() of previous run
process(noerrors)

# errors in initialize() -> fail
if os.fork() == 0:
    noerrors.add_module(ErrorInInit())
    process(noerrors)
else:
    assert os.wait()[1] != 0

# errors before process() -> fail
if os.fork() == 0:
    B2ERROR("htns")
    process(noerrors)
else:
    assert os.wait()[1] != 0

B2INFO('everything was OK.')
