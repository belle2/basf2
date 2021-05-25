#!/usr/bin/env python3

import basf2
from ROOT import Belle2
import time

basf2.set_random_seed("something important")
basf2.set_log_level(basf2.LogLevel.ERROR)


class DelayEvents(basf2.Module):
    """Delay second event in each run"""

    def event(self):
        """reimplementation of Module::event()."""

        evtmetadata = Belle2.PyStoreObj('EventMetaData')

        if not evtmetadata:
            basf2.B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            if event == 2:
                time.sleep(0.2)


# Normal steering file part begins here
nruns = 6
basf2.set_nprocesses(2)
main = basf2.Path()
main.add_module("EventInfoSetter", evtNumList=[2] * nruns, expList=[0] * nruns,
                runList=list(range(1, nruns + 1)))
test = main.add_module(DelayEvents())
test.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
main.add_module('Progress', logLevel=basf2.LogLevel.INFO)
basf2.process(main)
