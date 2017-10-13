#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2ERROR, B2FATAL, B2INFO
from .utils import AlgResult
from .utils import runs_overlapping_iov


class AlgorithmStrategyBase():
    """Base class for Algorithm strategies. These do the actual execution of
Algorithm Machine class on collected data. Each strategy may be quite different
in terms of how fast it may be, how database payloads are passed between executions, and
if the final payloads have an IoV that is independent to the actual runs used to calculates
them."""

    def __init__(self):
        self.results = []
        self.machine = None
        self.strategy_specific_args = {}
        self.iov_to_calibrate = None

    def run():
        raise NotImplementedError('Need to implement a run() method in {} Strategy.'.format(self.__class__.__name__))


class SingleIOV(AlgorithmStrategyBase):
    """The fastest and simplest Algorithm strategy. Runs the algorithm only once over all of the input
data or only the data corresponding to the requested IoV. The payload IoV is the set to the same as the one
that was executed.
"""

    def __init__(self):
        super().__init__()

    def run(self):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        B2INFO("Setting up SingleIOV strategy for ".format(self.machine.name))
        self.machine.setup_algorithm()
        B2INFO("Beginning execution of {}".format(self.machine.name))
        runs_to_execute = []
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if self.iov_to_calibrate:
            runs_to_execute = runs_overlapping_iov(self.iov_to_calibrate, self.machine.all_runs_collected)
        self.machine.execute_runs(runs=runs_to_execute)
        B2INFO("Finished execution with result code {}".format(self.machine.results[0].result))
        # Save the result
        self.results.append(self.machine.results[0])
        if (self.results[0].result == AlgResult.ok.value) or (self.results[0].result == AlgResult.iterate.value):
            # Commit all the payloads and send out the results
            self.machine.algorithm.algorithm.commit()
