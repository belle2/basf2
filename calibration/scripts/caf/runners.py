#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from abc import ABC, abstractmethod
from threading import Thread
import ROOT
from caf.utils import decode_json_string
from caf.state_machines import MachineError, ConditionError, TransitionError
from caf.state_machines import AlgorithmMachine
import basf2
from basf2 import B2ERROR, B2FATAL, B2INFO
import multiprocessing


class Runner(ABC):
    """Abstract Base Class for Runner type object"""
    @abstractmethod
    def run(self):
        """
        """
        pass


class AlgorithmsRunner(Runner):
    """
    Base class for `AlgorithmsRunner` classes. Defines the necessary information that will be provided to every
    `AlgorithmsRunner` used by the `framework.CAF`

    An `AlgorithmsRunner` will be given a list of `framework.Algorithm` objects defined during the setup of a
    `framework.Calibration` instance. The `AlgorithmsRunner` describes how to run each of the `strategies.AlgorithmStrategy`
    objects. As an example, assume that a single `framework.Calibration` was given and list of two `framework.Algorithm`
    instances to run.

    In this example the chosen :py:meth:`AlgorithmsRunner.run()` is simple and just loops over the list of `caf.framework.Algorithm`
    calling each one's :py:meth:`caf.strategies.AlgorithmStrategy.run()` methods in order.
    Thereby generating a localdb with the only communication between the `strategies.AlgorithmStrategy` instances coming from the
    database payloads being available from one algorithm to the next.

    But you could imagine a more complex situation. The `AlgorithmsRunner` might take the first `framework.Algorithm` and
    call its `AlgorithmStrategy.run` for only the first (exp,run) in the collected data. Then it might not commit the payloads
    to a localdb but instead pass some calculated values to the next algorithm to run on the same IoV. Then it might go back
    and re-run the first AlgorithmStrategy with new information and commit payloads this time. Then move onto the next IoV.

    Hopefully you can see that while the default provided `AlgorithmsRunner` and `AlgorithmStrategy` classes should be good for
    most situations, you have lot of freedom to define your own strategy if needed. By following the basic requirements for the
    interface to the `framework.CAF` you can easily plugin a different special case, or mix and match a custom class with
    default CAF ones.

    The run(self) method should be defined for every derived `AlgorithmsRunner`. It will be called once and only once for each
    iteration of (collector -> algorithm).

    Input files are automatically given via the `framework.Calibration.output_patterns` which constructs
    a list of all files in the collector output directories that match the output_patterns. If you have multiple types of
    output data it is your job to filter through the input files and assign them correctly.

    A list of local database paths are given to the `AlgorithmsRunner` based on the `framework.Calibration` dependencies and
    any overall localdb given to the CAF. By default you can call the "setup_algorithm" transition of the
    `caf.state_machines.AlgorithmMachine` to automatically set a database chain based on this list.
    But you have freedom to not call this at all in `run`, or to implement a different method to deal with this.
    """

    def __init__(self, name):
        """
        """
        #: The name of this runner instance
        self.name = name
        #: All of the output files made by the collector job and recovered by the "output_patterns"
        self.input_files = []
        #: User input local database, can be used to apply your own constants
        self.local_database_chain = []
        #: List of local databases created by previous CAF calibrations/iterations
        self.dependent_databases = []
        #: The directory of the local database we use to store algorithm payloads from this execution
        self.output_database_dir = ""
        #: Algorithm results from each algorithm we execute
        self.results = {}
        #: The list of algorithms that this runner executes
        self.algorithms = None
        #: Output directory of these algorithms, for logging mostly
        self.output_dir = ""


class SeqAlgorithmsRunner(AlgorithmsRunner):
    """
    """

    def __init__(self, name):
        """
        """
        super().__init__(name)

    def run(self, iov, iteration):
        """
        """
        B2INFO("SequentialAlgorithmsRunner begun for Calibration {}".format(self.name))
        # First we do the setup of algorithm strategies
        strategies = []
        for algorithm in self.algorithms:
            # Need to create an instance of the requested strategy and set the attributes
            strategy = algorithm.strategy(algorithm)
            strategy.input_files = self.input_files
            strategy.output_dir = self.output_dir
            strategy.output_database_dir = self.output_database_dir
            strategy.global_tag = self.global_tag
            strategy.local_database_chain = self.local_database_chain
            strategy.dependent_databases = self.dependent_databases
            strategies.append(strategy)

        parent_conn, child_conn = multiprocessing.Pipe()
        # We then fork off a copy of this python process so that we don't affect the original with logging changes
        ctx = multiprocessing.get_context("fork")
        for strategy in strategies:
            child = ctx.Process(target=SeqAlgorithmsRunner._run_strategy,
                                args=(strategy, iov, iteration, child_conn))
            child.start()
            child.join()
            self.results[algorithm.name] = parent_conn.recv()
        B2INFO("SequentialAlgorithmsRunner finished for Calibration {}".format(self.name))

    @staticmethod
    def _run_strategy(strategy, iov, iteration, conn):
        """Runs the AlgorithmStrategy sends back the results"""
        strategy.run(iov, iteration)
        # Get the return codes of the algorithm for the IoVs found by the Process
        conn.send(strategy.results)


class RunnerError(Exception):
    """
    Base exception class for Runners
    """
    pass
