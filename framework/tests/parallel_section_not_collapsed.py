#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Check that a lone StatisticsSummary cannot collapse the parallel section.

add_simulation() adds a parallel-processing-certified StatisticsSummary called
"Sum_PreSimulation" in front of BGOverlayInput, which is deliberately *not*
certified so that it runs before the fork. If that lone statistics module is
allowed to open a parallel section, the overlay input closes it again on the very
next module, and everything after it lands in the output path -- so the whole job
runs in a single process while still producing correct output. Nothing warns.

This reproduces that module ordering with stand-in modules and asserts that the
work afterwards really runs in a worker process. The path-arithmetic unit tests in
path_split.cc cover PathUtils::splitPath(), which only the ZMQ processor uses; the
default RingBuffer processor uses pEventProcessor::analyzePath(), which is private,
so this end-to-end check is the only way to pin its behaviour.
"""

import basf2
from ROOT import Belle2
from b2test_utils import clean_working_directory


class RecordProcessRole(basf2.Module):
    """Parallel-certified module that records which process role it ran in."""

    def __init__(self):
        """Flag ourselves as safe to run in a worker."""
        super().__init__()
        self.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        #: roles seen across all events
        self.roles = set()

    def event(self):
        """Remember the role of the process executing us."""
        self.roles.add(Belle2.ProcHandler.getProcessName())

    def terminate(self):
        """Fail unless we ran in a worker.

        terminate() runs in every process that holds the module, so only assert
        where we actually saw events.
        """
        if not self.roles:
            return
        if self.roles != {"worker"}:
            basf2.B2FATAL(
                "A parallel-certified module ran outside a worker process: saw "
                f"{sorted(self.roles)}. The parallel section collapsed, so this job "
                "would run single-process despite -p."
            )


class NotParallelCertified(basf2.Module):
    """Stand-in for BGOverlayInput: must run before the fork, so not certified."""

    def event(self):
        """Nothing to do; only the property flags matter here."""


main = basf2.Path()
main.add_module("EventInfoSetter", evtNumList=[4])

# This is the ordering that add_simulation() produces.
main.add_module("StatisticsSummary").set_name("Sum_PreSimulation")
main.add_module(NotParallelCertified())

recorder = RecordProcessRole()
main.add_module(recorder)

# Guard the assumptions the test rests on, so a flag change elsewhere shows up
# here as a clear message rather than as a confusing failure below.
stats = [m for m in main.modules() if m.type() == "StatisticsSummary"][0]
if not stats.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED):
    basf2.B2FATAL("StatisticsSummary is no longer parallel certified; this test needs rethinking")
if not stats.has_properties(basf2.ModulePropFlags.NOPARALLELWORK):
    basf2.B2FATAL("StatisticsSummary lost its NOPARALLELWORK flag, which is what keeps it from "
                  "opening a parallel section of its own")
if not recorder.has_properties(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED):
    basf2.B2FATAL("the recorder module failed to set its parallel certification")

# one worker is enough: this still forks input, worker and output, so the
# same code path is exercised, and b2test-scripts runs tests in parallel
basf2.set_nprocesses(1)
with clean_working_directory():
    basf2.process(main)
