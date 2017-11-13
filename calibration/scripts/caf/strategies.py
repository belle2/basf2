#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2ERROR, B2FATAL, B2INFO
from .utils import AlgResult
from .utils import runs_overlapping_iov
from .utils import runs_from_vector
from abc import ABC, abstractmethod
from caf.state_machines import AlgorithmMachine


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

    def __init__(self, algorithm):
        """
        """
        #: Algorithm() class that we're running
        self.algorithm = algorithm
        #: The results dictionary that should be filled as the algorithm gets run
        self.results = []
        #: Collector output files, will contain all files retured by the output patterns
        self.input_files = []
        #: The algorithm output directory which is mostly used to store the stdout file
        self.output_dir = ""
        #: The output database directory for the localdb that the algorithm will commit to
        self.output_database_dir = ""
        #: The global tag that has been used for this calibration, empty string means no gloabl tag
        self.global_tag = ""
        #: User defined local database chain i.e. if you have localdb's for custom alignment etc
        self.local_database_chain = []
        #: CAF created local databases from previous calibrations that this calibration/algorithm depends on
        self.dependent_databases = []

    @abstractmethod
    def run(self, iov, iteration):
        """
        Abstract method that needs to be implemented. It will be called to actually execute the
        algorithm.
        """
        pass


class SingleIOV(AlgorithmStrategy):
    """The fastest and simplest Algorithm strategy. Runs the algorithm only once over all of the input
    data or only the data corresponding to the requested IoV. The payload IoV is the set to the same as the one
    that was executed.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: Create a basic (mostly empty) AlgorithmMachine, gets setup properly in run()
        self.machine = AlgorithmMachine(self.algorithm)

    def run(self, iov, iteration):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        B2INFO("Setting up SingleIOV strategy for {}".format(self.algorithm.name))
        self.machine.global_tag = self.global_tag
        self.machine.local_database_chain = self.local_database_chain
        self.machine.dependent_databases = self.dependent_databases
        self.machine.output_dir = self.output_dir
        self.machine.output_database_dir = self.output_database_dir
        self.machine.input_files = self.input_files
        self.machine.setup_algorithm(iteration=iteration)
        B2INFO("Beginning execution of {}".format(self.algorithm.name))
        runs_to_execute = []
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        self.machine.execute_runs(runs=runs_to_execute, iteration=iteration)
        B2INFO("Finished execution with result code {}".format(self.machine.result.result))
        # Save the result
        self.results.append(self.machine.result)
        if (self.results[0].result == AlgResult.ok.value) or (self.results[0].result == AlgResult.iterate.value):
            # Commit all the payloads and send out the results
            self.machine.algorithm.algorithm.commit()
#
#
# class SequentialRuns(AlgorithmStrategy):
#     """
#     """
#
#     def __init__(self, algorithm):
#         super().__init__(algorithm)
#         #: Create a basic (mostly empty) AlgorithmMachine, gets setup properly in run()
#
#         self.machine = AlgorithmMachine(self.algorithm)
#
#     def run(self, iov, iteration):
#         """
#         Runs the algorithm machine over the collected data and fills the results.
#         """
#         B2INFO("Setting up SequentialRuns strategy for ".format(self.algorithm.name))
#         self.machine.global_tag = self.global_tag
#         self.machine.local_database_chain = self.local_database_chain
#         self.machine.dependent_databases = self.dependent_databases
#         self.machine.output_dir = self.output_dir
#         self.machine.output_database_dir = self.output_database_dir
#         self.machine.input_files = self.input_files
#         self.machine.setup_algorithm()
#         B2INFO("Beginning execution of {}".format(self.algorithm.name))
#         runs_to_execute = []
#         # If we were given a specific IoV to calibrate we just execute over runs in that IoV
#         if iov:
#             all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
#             runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
#         # The runs we have left to execute
#         remaining_runs = runs_to_execute[:]
#         # The previous execution's runs
#         previous_runs = []
#         # The current runs we are executing
#         current_runs = []
#         for exprun in runs_to_execute:
#             # Add on the next run
#             current_runs.append(exprun)
#             # Remove it from our remaining runs
#             remaining_runs.pop(0)
#             B2INFO("Executing on IoV = {}".format(iov_from_runs(current_runs)))
#             self.machine.execute_runs(runs=current_runs, iteration=iteration)
#             B2INFO("Finished execution with result code {}".format(self.machine.result.result))
#             # Does this count as a successful execution?
#             if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
#                 # Commit all the payloads and send out the results
#                 self.machine.algorithm.algorithm.commit()
#                 # Save the result
#                 self.results.append(self.machine.result)
#                 previous_runs = current_runs[:]
#                 current_runs = []
#             # If it wasn't successful, was it due to lack of data in the runs?
#             elif (self.machine.result.result == AlgResult.not_enough_data.value):
#                 B2INFO("There wasn't enough data in the IoV {}".format(iov_from_runs(current_runs)))
#                 if remaining_runs:
#                     B2INFO("Some runs remain to be processed will try to merge the IoV with them")
#                 else:
#                     B2INFO("There aren't any more runs remaining to merge with. So we'll merge with hte previous IoV.")
#                     current_runs = previous_runs.extend(current_runs)
#         else:
#             # Check if we need to run a final execution on the previous execution + dangling set of runs
#             if current_runs:
#                 self.machine.execute_runs(runs=current_runs, iteration=iteration)
#                 if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
#                    # Commit all the payloads and send out the results
#                    self.machine.algorithm.algorithm.commit()
#                    # Save the result
#                    self.results.append(self.machine.result)
#
