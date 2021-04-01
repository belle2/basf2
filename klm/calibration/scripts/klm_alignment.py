#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Custom calibration strategy for KLM alignment."""

from basf2 import B2INFO
from caf.strategies import SequentialRunByRun, StrategyError
from caf.utils import ExpRun, IoV
from caf.utils import split_runs_by_exp
from caf.utils import runs_overlapping_iov, runs_from_vector
import json


class KLMAlignment(SequentialRunByRun):
    """
    Custom stragery for KLM alignment. Based on SequentialRunByRun strategy,
    but requires custom experiment-dependent settings.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {
        "iov_coverage": IoV,
        "millepede_entries": int,
        "millepede_entries_exp7": int,
        "step_size": int
    }

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        self.millepede_entries = 0
        self.millepede_entries_exp7 = 0

    def run(self, iov, iteration, queue):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        self.queue = queue
        B2INFO(f"Setting up {self.__class__.__name__} strategy for {self.algorithm.name}.")
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["database_chain"] = self.database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        machine_params["ignored_runs"] = self.ignored_runs
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO(f"Beginning execution of {self.algorithm.name} using strategy {self.__class__.__name__}.")
        runs_to_execute = []
        all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
        # If we were given a specific IoV to calibrate we just execute over runs in that IoV
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected[:]

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            B2INFO(f"Removing the ignored_runs from the runs to execute for {self.algorithm.name}.")
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)

        # We don't want to cross the boundary of Experiments accidentally. So we will split our run list
        # into separate lists, one for each experiment number contained. That way we can evaluate each experiment
        # separately and prevent IoVs from crossing the boundary.
        runs_to_execute = split_runs_by_exp(runs_to_execute)

        # Now iterate through the experiments, executing runs in blocks of 'step_size'. We DO NOT allow a payload IoV to
        # extend over multiple experiments, only multiple runs
        iov_coverage = None
        if "iov_coverage" in self.algorithm.params:
            B2INFO(f"Detected that you have set iov_coverage to {self.algorithm.params['iov_coverage']}.")
            iov_coverage = self.algorithm.params["iov_coverage"]

        if "millepede_entries" in self.algorithm.params:
            self.millepede_entries = self.algorithm.params["millepede_entries"]

        if "millepede_entries_exp7" in self.algorithm.params:
            self.millepede_entries_exp7 = self.algorithm.params["millepede_entries_exp7"]

        number_of_experiments = len(runs_to_execute)
        # Iterate over experiment run lists
        for i_exp, run_list in enumerate(runs_to_execute, start=1):
            # If 'iov_coverage' was set in the algorithm.params and it is larger (at both ends) than the
            # input data runs IoV, then we also have to set the first payload IoV to encompass the missing beginning
            # of the iov_coverage, and the last payload IoV must cover up to the end of iov_coverage.
            # This is only true for the lowest and highest experiments in our input data.
            if run_list[0].exp == 7:
                self.machine.algorithm.algorithm.setMinEntries(self.millepede_entries_exp7)
            else:
                self.machine.algorithm.algorithm.setMinEntries(self.millepede_entries)
            if iov_coverage and i_exp == 1:
                lowest_exprun = ExpRun(iov_coverage.exp_low, iov_coverage.run_low)
            else:
                lowest_exprun = run_list[0]

            if iov_coverage and i_exp == number_of_experiments:
                highest_exprun = ExpRun(iov_coverage.exp_high, iov_coverage.run_high)
            else:
                highest_exprun = run_list[-1]

            self.execute_over_run_list(iteration, run_list, lowest_exprun, highest_exprun)

        # Print any knowable gaps between result IoVs, if any are foun there is a problem.
        gaps = self.find_iov_gaps()
        # Dump them to a file for logging
        with open(f"{self.algorithm.name}_iov_gaps.json", "w") as f:
            json.dump(gaps, f)

        # If any results weren't successes we fail
        if self.any_failed_iov():
            self.send_final_state(self.FAILED)
        else:
            self.send_final_state(self.COMPLETED)
