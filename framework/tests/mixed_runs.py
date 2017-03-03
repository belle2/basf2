#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
import time

set_random_seed("something important")
set_log_level(LogLevel.ERROR)


class DelayEvents(Module):
    """Delay second event in each run"""

    def event(self):
        """reimplementation of Module::event()."""

        evtmetadata = Belle2.PyStoreObj('EventMetaData')

        if not evtmetadata:
            B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            if event == 2:
                time.sleep(0.2)

# Normal steering file part begins here

nruns = 6
set_nprocesses(2)
main = create_path()
main.add_module("EventInfoSetter", evtNumList=[2] * nruns, expList=[0] * nruns,
                runList=list(range(1, nruns + 1)))
test = main.add_module(DelayEvents())
test.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
main.add_module('Progress', logLevel=LogLevel.INFO)
process(main)
