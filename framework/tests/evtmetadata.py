#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_random_seed("something important")


class NoopModule(Module):

    """Doesn't do anything."""


class EvtMetaDataTest(Module):

    """Prints EventMetaData objects and stops event processing in event 3."""

    def __init__(self):
        """constructor."""

        # need to call super() _if_ we reimplement the constructor
        super(EvtMetaDataTest, self).__init__()
        B2INFO('event\trun\texp')

    def initialize(self):
        """reimplementation of Module::initialize()."""

        B2INFO('EvtMetaDataTest::initialize()')

    def beginRun(self):
        """reimplementation of Module::beginRun()."""

        B2INFO('EvtMetaDataTest::beginRun()')

    def event(self):
        """reimplementation of Module::event()."""

        evtmetadata = Belle2.PyStoreObj('EventMetaData')

        if not evtmetadata:
            B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            run = evtmetadata.obj().getRun()
            exp = evtmetadata.obj().getExperiment()
            B2INFO(str(event) + '\t' + str(run) + '\t' + str(exp))
            if event == 4:
                # stop event processing.
                evtmetadata.obj().setEndOfData()

    def endRun(self):
        """reimplementation of Module::endRun()."""

        B2INFO('EvtMetaDataTest::endRun()')

    def terminate(self):
        """reimplementation of Module::terminate()."""

        B2INFO('EvtMetaDataTest::terminate()')


# Normal steering file part begins here

for skipNEvents in range(10):
    # Create main path
    main = create_path()

    # not used for anything, just checking wether the master module
    # can be found if it's not the first module in the path.
    main.add_module(NoopModule())

    # exp 0 has only 2 events, so cannot trigger the test module
    # also tests for a problem where beginRun() wasn't called
    # exp 1 has only 2 events, so cannot trigger the test module,
    # exp 2 has no events and will be skipped
    # exp 3 will be stopped in event 3 by EvtMetaDataTest
    main.add_module('EventInfoSetter',
                    expList=[0, 1, 2, 3],
                    runList=[0, 1, 2, 3],
                    evtNumList=[2, 2, 0, 5],
                    skipNEvents=skipNEvents)

    main.add_module(EvtMetaDataTest())

    process(main)


# test skipping to specific events
skipToEvents = [
    [0, 0, 1],
    [0, 0, 3],
    [0, 1, 1],
    [0, 1, 6],
    [0, 2, 3],
    [1, 0, 1],
    [2, 0, 1],
]

for event in skipToEvents:
    B2INFO("skipping to exp={0}, run={1}, evt={2}".format(*event))
    main = create_path()
    main.add_module("EventInfoSetter", evtNumList=[3, 3, 3, 3], expList=[0, 0, 0, 1],
                    runList=[0, 1, 2, 0], skipToEvent=event)
    main.add_module(EvtMetaDataTest())
    process(main)
