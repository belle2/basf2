#!/usr/bin/env python

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import modularAnalysis as ma


def add_gdl_trigger(path, SimulationMode=1, FilterEvents=False, simulateT0jitter=False, Belle2Phase="Phase2"):
    """
    add the gdl module to path
    @param path            module is added to this path
    @param SimulationMode  the simulation mode in TSIM, 1: fast simulation,
                           trigger algorithm simulation only, no firmware simulation
                           2: full simulation, both trigger algorithm and firmware
                           are simulated
    @param FilterEvents    if True only the events that pass the L1 trigger will
                           survive simulation, the other are discarded.
                           Make sure you do need to filter events before you set
                           the value to True.
    """
    trggdl = b2.register_module('TRGGDL')
    trggdl.param('SimulationMode', SimulationMode)
    trggdl.param('Belle2Phase', Belle2Phase)
    trggdl.param('simulateT0jitter', simulateT0jitter)
    path.add_module(trggdl)
    if FilterEvents:
        ma.applyEventCuts('L1Trigger == 1', path)


class TRGAbortGapFilter(b2.Module):
    """
    Reject events whose `coml1rvc` lies inside configured abort gaps.
    The same nomenclature used in ``TRGGDLDQM`` module is used here for easy comparison of the code/logic.
    """

    def initialize(self):
        """Initialize."""
        import ROOT
        self.entAry = ROOT.Belle2.PyStoreArray("TRGGDLUnpackerStores")
        self.entAry.isRequired()
        self.bevt = ROOT.Belle2.PyStoreObj("EventMetaData")
        self.bevt.isRequired()
        self.m_unpacker = ROOT.Belle2.PyDBObj("TRGGDLDBUnpacker")
        self.abort_gaps = {  # {experiment: [(min, max), ...]}
            37: [
                (544, 586),   # gap 1
                (1183, 1225)  # gap 2
                ],
            39: [
                (316, 358),   # gap 1
                (956, 998)    # gap 2
                ]
        }

    def beginRun(self):
        """Begin run."""
        if not self.bevt.isValid():
            b2.B2FATAL("No valid EventMetaData object")

        self.exp = self.bevt.getExperiment()
        self.run = self.bevt.getRun()

        self.run_filter = self.exp in self.abort_gaps
        if not self.run_filter:
            return

        if not self.m_unpacker.isValid():
            b2.B2FATAL("No valid payload for TRGGDLDBUnpacker")

        self._e_coml1rvc = None
        for i in range(320):
            if self.m_unpacker.getLeafnames(i) == "coml1rvc":
                self._e_coml1rvc = i
                break

        if not self._e_coml1rvc:
            b2.B2FATAL(
                "No leaf named `coml1rvc` in TRGGDLDBUnpacker payload "
                f"in exp={self.exp} and run={self.run}"
            )

    def event(self):
        """Event."""
        self.return_value(1)

        if not self.run_filter:
            return

        if (self.entAry.getEntries() == 0):
            return

        store = self.entAry[0]
        coml1rvc = store.m_unpacker[self._e_coml1rvc]

        for abort_gap_min, abort_gap_max in self.abort_gaps[self.exp]:
            if abort_gap_min <= coml1rvc < abort_gap_max:
                self.return_value(0)


def filter_trigger_abort_gaps(path):
    """
    Add the ``TRGAbortGapFilter`` module and the relevant logic to the ``path`` to filter the events
    in the trigger abort gaps.
    """

    empty = b2.Path()
    abort_gap_filter = path.add_module(TRGAbortGapFilter())
    abort_gap_filter.if_value('<1', empty)
