#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2ERROR, B2FATAL, B2INFO
from .utils import AlgResult
from .utils import runs_overlapping_iov
from abc import ABC, abstractmethod


class AlgorithmStrategy(ABC):
    """Base class for Algorithm strategies. These do the actual execution of a single
    algorithm on collected data. Each strategy may be quite different in terms of how fast it may be,
    how database payloads are passed between executions, and whether or not final payloads have an IoV
    that is independent to the actual runs used to calculates them.

    This base class defines the basic attributes and methods that will be automatically used by the selected AlgorithmRunner.
    When defining a derived class you are free to use these attributes or to implement as much functionality as you want.

    If you define your derived class with an __init__ method, then you should first call the base class
    `AlgorithmStrategy.__init__`  method via super() e.g.

    >>> def __init__(self):
    >>>     super().__init__()

    The most important method to implement is a `AlgorithmStrategy.run` method which will take an algorithm and execute it
    in the required way defined by the options you have selected/attributes set.
    """

    def __init__(self):
        self.results = []
        self.machine = None
        self.strategy_specific_args = {}
        self.iov_to_calibrate = None

    @abstractmethod
    def run(self):
        pass


class SingleIOV(AlgorithmStrategy):
    """The fastest and simplest Algorithm strategy. Runs the algorithm only once over all of the input
    data or only the data corresponding to the requested IoV. The payload IoV is the set to the same as the one
    that was executed.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
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
