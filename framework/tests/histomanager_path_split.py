#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Check that merging a pointless parallel section back leaves the histogram manager alone.

The histogram manager has to run in every process, so the path splitting puts it on all
three paths. A parallel section made up of nothing but c_NoParallelWork modules is
merged back into the input path -- and that section used to contain the histogram
manager, which was thereby duplicated into the input path and dropped from the main
path, leaving the workers filling no histograms at all.

This is the DQM version of parallel_section_not_collapsed.py: the module ordering of
add_simulation(), with a histogram manager in front. path_split.cc covers
PathUtils::splitPath(), which only the ZMQ processor uses; the default RingBuffer
processor uses pEventProcessor::analyzePath(), which is private, so the paths it builds
are checked here through the lines it logs for them.
"""

import re

import basf2
from b2test_utils import clean_working_directory, run_in_subprocess

LOG_FILE = "path_split.log"


class NotParallelCertified(basf2.Module):
    """Stand-in for BGOverlayInput: must run before the fork, so not certified."""

    def event(self):
        """Nothing to do; only the property flags matter here."""


class ParallelWork(basf2.Module):
    """Stand-in for the real work, which is what the parallel section is for."""

    def __init__(self):
        """Flag ourselves as safe to run in a worker."""
        super().__init__()
        self.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

    def event(self):
        """Nothing to do; only the property flags matter here."""


def run():
    """Process the DQM-style path with one worker, logging the split paths to a file."""
    basf2.logging.add_file(LOG_FILE)
    basf2.set_log_level(basf2.LogLevel.INFO)

    main = basf2.Path()
    main.add_module("HistoManager", histoFileName="histos.root")
    main.add_module("EventInfoSetter", evtNumList=[4])
    # This is the ordering that add_simulation() produces.
    main.add_module("StatisticsSummary").set_name("Sum_PreSimulation")
    main.add_module(NotParallelCertified())
    main.add_module(ParallelWork())

    basf2.set_nprocesses(1)
    basf2.process(main)


def path_line(log, which):
    """The modules pEventProcessor logged for the given path, as a list of names."""
    for line in log:
        match = re.search(rf"{which} Path \[(.*)\]", line)
        if match:
            # addPath() nests, so the path string can carry brackets around a sub-path
            return [name.strip(" []") for name in match.group(1).split("->")]
    basf2.B2FATAL(f"pEventProcessor logged no {which} path; did the job fall back to single process?")


with clean_working_directory():
    if run_in_subprocess(target=run) != 0:
        basf2.B2FATAL("the job itself failed, so there is nothing to check")

    with open(LOG_FILE) as log_file:
        log = log_file.readlines()

    for which in ("Input", "Main", "Output"):
        modules = path_line(log, which)
        count = modules.count("HistoManager")
        if which == "Input":
            if count != 1:
                basf2.B2FATAL(f"the histogram manager appears {count} times in the input path {modules}; "
                              "it would be initialised once per copy")
        elif count != 1:
            basf2.B2FATAL(f"the histogram manager is missing from the {which.lower()} path {modules}, "
                          "so that process would fill no histograms")
