#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import B2DEBUG, B2ERROR, B2INFO, B2WARNING
from caf.utils import AlgResult
from caf.utils import B2INFO_MULTILINE
from caf.utils import runs_overlapping_iov, runs_from_vector
from caf.utils import iov_from_runs, split_runs_by_exp, vector_from_runs
from caf.utils import find_gaps_in_iov_list, grouper, find_run_lists_from_boundaries
from caf.utils import IoV, ExpRun
from caf.state_machines import AlgorithmMachine

from abc import ABC, abstractmethod
import json


class AlgorithmStrategy(ABC):
    """
    Base class for Algorithm strategies. These do the actual execution of a single
    algorithm on collected data. Each strategy may be quite different in terms of how fast it may be,
    how database payloads are passed between executions, and whether or not final payloads have an IoV
    that is independent to the actual runs used to calculates them.

    Parameters:
        algorithm (:py:class:`caf.framework.Algorithm`): The algorithm we will run

    This base class defines the basic attributes and methods that will be automatically used by the selected AlgorithmRunner.
    When defining a derived class you are free to use these attributes or to implement as much functionality as you want.

    If you define your derived class with an __init__ method, then you should first call the base class
    `AlgorithmStrategy.__init__()`  method via super() e.g.

    >>> def __init__(self):
    >>>     super().__init__()

    The most important method to implement is :py:meth:`AlgorithmStrategy.run` which will take an algorithm and execute it
    in the required way defined by the options you have selected/attributes set.
    """
    #: Required attributes that must exist before the strategy can run properly.
    #: Some are allowed be values that return False whe tested e.g. "" or []
    required_attrs = ["algorithm",
                      "database_chain",
                      "dependent_databases",
                      "output_dir",
                      "output_database_dir",
                      "input_files",
                      "ignored_runs"
                      ]

    #: Attributes that must have a value that returns True when tested by :py:meth:`is_valid`.
    required_true_attrs = ["algorithm",
                           "output_dir",
                           "output_database_dir",
                           "input_files"
                           ]

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run", "all"]

    #: Signal value that is put into the Queue when there are no more results left
    FINISHED_RESULTS = "DONE"

    #: Completed state
    COMPLETED = "COMPLETED"

    #: Failed state
    FAILED = "FAILED"

    def __init__(self, algorithm):
        """
        """
        #: Algorithm() class that we're running
        self.algorithm = algorithm
        #: Collector output files, will contain all files retured by the output patterns
        self.input_files = []
        #: The algorithm output directory which is mostly used to store the stdout file
        self.output_dir = ""
        #: The output database directory for the localdb that the algorithm will commit to
        self.output_database_dir = ""
        #: User defined database chain i.e. the default global tag, or if you have localdb's/tags for custom alignment etc
        self.database_chain = []
        #: CAF created local databases from previous calibrations that this calibration/algorithm depends on
        self.dependent_databases = []
        #: Runs that will not be included in ANY execution of the algorithm. Usually set by Calibration.ignored_runs.
        #: The different strategies may handle the resulting run gaps differently.
        self.ignored_runs = []
        #: The list of results objects which will be sent out before the end
        self.results = []
        #: The multiprocessing Queue we use to pass back results one at a time
        self.queue = None

    @abstractmethod
    def run(self, iov, iteration, queue):
        """
        Abstract method that needs to be implemented. It will be called to actually execute the
        algorithm.
        """

    def setup_from_dict(self, params):
        """
        Parameters:
            params (dict): Dictionary containing values to be assigned to the strategy attributes of the same name.
        """
        for attribute_name, value in params.items():
            setattr(self, attribute_name, value)

    def is_valid(self):
        """
        Returns:
            bool: Whether or not this strategy has been set up correctly with all its necessary attributes.
        """
        B2INFO("Checking validity of current AlgorithmStrategy setup.")
        # Check if we're somehow missing a required attribute (should be impossible since they get initialised in init)
        for attribute_name in self.required_attrs:
            if not hasattr(self, attribute_name):
                B2ERROR(f"AlgorithmStrategy attribute {attribute_name} doesn't exist.")
                return False
        # Check if any attributes that need actual values haven't been set or were empty
        for attribute_name in self.required_true_attrs:
            if not getattr(self, attribute_name):
                B2ERROR(f"AlgorithmStrategy attribute {attribute_name} returned False.")
                return False
        return True

    def find_iov_gaps(self):
        """
        Finds and prints the current gaps between the IoVs of the strategy results. Basically these are the IoVs
        not covered by any payload. It CANNOT find gaps if they exist across an experiment boundary. Only gaps
        within the same experiment are found.

        Returns:
            iov_gaps(list[IoV])
        """
        iov_gaps = find_gaps_in_iov_list(sorted([result.iov for result in self.results]))
        if iov_gaps:
            gap_msg = ["Found gaps between IoVs of algorithm results (regardless of result)."]
            gap_msg.append("You may have requested these gaps deliberately by not passing in data containing these runs.")
            gap_msg.append("This may not be a problem, but you will not have payoads defined for these IoVs")
            gap_msg.append("unless you edit the final database.txt yourself.")
            B2INFO_MULTILINE(gap_msg)
            for iov in iov_gaps:
                B2INFO(f"{iov} not covered by any execution of the algorithm.")
        return iov_gaps

    def any_failed_iov(self):
        """
        Returns:
            bool: If any result in the current results list has a failed algorithm code we return True
        """
        failed_results = []
        for result in self.results:
            if result.result == AlgResult.failure.value or result.result == AlgResult.not_enough_data.value:
                failed_results.append(result)
        if failed_results:
            B2WARNING("Failed results found.")
            for result in failed_results:
                if result.result == AlgResult.failure.value:
                    B2ERROR(f"c_Failure returned for {result.iov}.")
                elif result.result == AlgResult.not_enough_data.value:
                    B2WARNING(f"c_NotEnoughData returned for {result.iov}.")
            return True
        else:
            return False

    def send_result(self, result):
        self.queue.put({"type": "result", "value": result})

    def send_final_state(self, state):
        self.queue.put({"type": "final_state", "value": state})


class SingleIOV(AlgorithmStrategy):
    """The fastest and simplest Algorithm strategy. Runs the algorithm only once over all of the input
    data or only the data corresponding to the requested IoV. The payload IoV is the set to the same as the one
    that was executed.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {"apply_iov": IoV}

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

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
        B2INFO(f"Starting AlgorithmMachine of {self.algorithm.name}.")
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO(f"Beginning execution of {self.algorithm.name} using strategy {self.__class__.__name__}.")

        all_runs_collected = set(runs_from_vector(self.algorithm.algorithm.getRunListFromAllData()))
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            B2INFO(f"Removing the ignored_runs from the runs to execute for {self.algorithm.name}.")
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)
        apply_iov = None
        if "apply_iov" in self.algorithm.params:
            apply_iov = self.algorithm.params["apply_iov"]
        self.machine.execute_runs(runs=runs_to_execute, iteration=iteration, apply_iov=apply_iov)
        B2INFO(f"Finished execution with result code {self.machine.result.result}.")

        # Send out the result to the runner
        self.send_result(self.machine.result)

        # Make sure the algorithm state and commit is done
        if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
            # Valid exit codes mean we can complete properly
            self.machine.complete()
            # Commit all the payloads and send out the results
            self.machine.algorithm.algorithm.commit()
            self.send_final_state(self.COMPLETED)
        else:
            # Either there wasn't enough data or the algorithm failed
            self.machine.fail()
            self.send_final_state(self.FAILED)


class SequentialRunByRun(AlgorithmStrategy):
    """
    Algorithm strategy to do run-by-run calibration of collected data.
    Runs the algorithm over the input data contained within the requested IoV, starting with the first run's data only.
    If the algorithm returns 'not enough data' on the current run set, it won't commit the payloads, but instead adds
    the next run's data and tries again.

    Once an execution on a set of runs return 'iterate' or 'ok' we move onto the next runs (if any are left)
    and start the same procedure again. Committing of payloads to the outputdb only happens once we're sure that there
    is enough data in the remaining runs to get a full execution. If there isn't enough data remaining, the last runs
    are merged with the previous successful execution's runs and a final execution is performed on all remaining runs.

    Additionally this strategy will automatically make sure that IoV gaps in your input data are covered by a payload.
    This means that there shouldn't be any IoVs that don't get a new payload by the  end of runnning an iteration.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {
        "has_experiment_settings": bool,
        "iov_coverage": IoV,
        "step_size": int
    }

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)
        if "step_size" not in self.algorithm.params:
            self.algorithm.params["step_size"] = 1
        self.first_execution = True

    def apply_experiment_settings(self, algorithm, experiment):
        """
        Apply experiment-dependent settings.
        This is the default version, which does not do anything.
        If necessary, it should be reimplemented by derived classes.
        """
        return

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

        number_of_experiments = len(runs_to_execute)
        # Iterate over experiment run lists
        for i_exp, run_list in enumerate(runs_to_execute, start=1):

            # Apply experiment-dependent settings.
            if "has_experiment_settings" in self.algorithm.params:
                if self.algorithm.params["has_experiment_settings"]:
                    self.apply_experiment_settings(self.machine.algorithm.algorithm, run_list[0].exp)

            # If 'iov_coverage' was set in the algorithm.params and it is larger (at both ends) than the
            # input data runs IoV, then we also have to set the first payload IoV to encompass the missing beginning
            # of the iov_coverage, and the last payload IoV must cover up to the end of iov_coverage.
            # This is only true for the lowest and highest experiments in our input data.
            # If we have multiple experiments the iov must be adjusted to avoid gaps at the iov boundaries
            lowest_exprun = ExpRun(run_list[0].exp, 0)
            highest_exprun = ExpRun(run_list[-1].exp, -1)

            if i_exp == 1:
                lowest_exprun = ExpRun(iov_coverage.exp_low, iov_coverage.run_low) if iov_coverage else run_list[0]
            if i_exp == number_of_experiments:
                highest_exprun = ExpRun(iov_coverage.exp_high, iov_coverage.run_high) if iov_coverage else run_list[-1]

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

    def execute_over_run_list(self, iteration, run_list, lowest_exprun, highest_exprun):
        # The runs (data) we have left to execute from this run list
        remaining_runs = run_list[:]
        # The previous execution's runs
        previous_runs = []
        # The current runs we are executing
        current_runs = []
        # The last successful payload and result
        last_successful_payloads = None
        last_successful_result = None

        # Iterate over ExpRuns within an experiment in chunks of 'step_size'
        for expruns in grouper(self.algorithm.params["step_size"], run_list):
            # Already set up earlier the first time, so we shouldn't do it again
            if not self.first_execution:
                self.machine.setup_algorithm()
            else:
                self.first_execution = False

            # Add on the next step of runs
            current_runs.extend(expruns)
            # Remove them from our remaining runs
            remaining_runs = [run for run in remaining_runs if run not in current_runs]

            # Is this the first payload of the experiment
            if not last_successful_result:
                B2INFO("Detected that this will be the first payload of this experiment.")
                # If this is the first payload but we have other data, we need the IoV to cover from the
                # lowest IoV extent requested up to the ExpRun right before the next run in the remaining runs list.
                if remaining_runs:
                    apply_iov = IoV(*lowest_exprun, remaining_runs[0].exp, remaining_runs[0].run - 1)
                # If this is the first payload but there isn't more data, we set the IoV to cover the full range
                else:
                    B2INFO("Detected that this will be the only payload of the experiment.")
                    apply_iov = IoV(*lowest_exprun, *highest_exprun)
            # If there were previous successes
            else:
                if not remaining_runs:
                    B2INFO("Detected that there are no more runs to execute in this experiment after this next execution.")
                    apply_iov = IoV(*current_runs[0], *highest_exprun)
                # Othewise, it's just a normal IoV in the middle.
                else:
                    B2INFO("Detected that there are more runs to execute in this experiment after this next execution.")
                    apply_iov = IoV(*current_runs[0], remaining_runs[0].exp, remaining_runs[0].run - 1)

            B2INFO(f"Executing and applying {apply_iov} to the payloads.")
            self.machine.execute_runs(runs=current_runs, iteration=iteration, apply_iov=apply_iov)
            B2INFO(f"Finished execution with result code {self.machine.result.result}.")

            # Does this count as a successful execution?
            if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                self.machine.complete()
                # If we've succeeded but we have a previous success we can commit the previous payloads
                # since we have new ones ready
                if last_successful_payloads and last_successful_result:
                    B2INFO("Saving this execution's payloads to be committed later.")
                    # Save the payloads and result
                    new_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                    new_successful_result = self.machine.result
                    B2INFO("We just succeded in execution of the Algorithm."
                           f" Will now commit payloads from the previous success for {last_successful_result.iov}.")
                    self.machine.algorithm.algorithm.commit(last_successful_payloads)
                    self.results.append(last_successful_result)
                    self.send_result(last_successful_result)
                    # If there are remaining runs we need to have the current payloads ready to commit after the next execution
                    if remaining_runs:
                        last_successful_payloads = new_successful_payloads
                        last_successful_result = new_successful_result
                    # If there's not more runs to process we should also commit the new ones
                    else:
                        B2INFO("We have no more runs to process. "
                               f"Will now commit the most recent payloads for {new_successful_result.iov}.")
                        self.machine.algorithm.algorithm.commit(new_successful_payloads)
                        self.results.append(new_successful_result)
                        self.send_result(new_successful_result)
                        break
                # if there's no previous success this must be the first run executed
                else:
                    # Need to save payloads for later if we have a success but runs remain
                    if remaining_runs:
                        B2INFO(f"Saving the most recent payloads for {self.machine.result.iov} to be committed later.")
                        # Save the payloads and result
                        last_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                        last_successful_result = self.machine.result
                    # Need to commit and exit if we have a success and no remaining data
                    else:
                        B2INFO("We just succeeded in execution of the Algorithm."
                               " No runs left to be processed, so we are committing results of this execution.")
                        self.machine.algorithm.algorithm.commit()
                        self.results.append(self.machine.result)
                        self.send_result(self.machine.result)
                        break

                previous_runs = current_runs[:]
                current_runs = []
            # If it wasn't successful, was it due to lack of data in the runs?
            elif (self.machine.result.result == AlgResult.not_enough_data.value):
                B2INFO(f"There wasn't enough data in {self.machine.result.iov}.")
                if remaining_runs:
                    B2INFO("Some runs remain to be processed. "
                           f"Will try to add at most {self.algorithm.params['step_size']} more runs of data and execute again.")
                elif not remaining_runs and not last_successful_result:
                    B2ERROR("There aren't any more runs remaining to merge with, and we never had a previous success."
                            " There wasn't enough data in the full input data requested.")
                    self.results.append(self.machine.result)
                    self.send_result(self.machine.result)
                    self.machine.fail()
                    break
                elif not remaining_runs and last_successful_result:
                    B2INFO("There aren't any more runs remaining to merge with. But we had a previous success"
                           ", so we'll merge with the previous IoV.")
                    final_runs = current_runs[:]
                    current_runs = previous_runs
                    current_runs.extend(final_runs)
                self.machine.fail()
            elif self.machine.result.result == AlgResult.failure.value:
                B2ERROR(f"{self.algorithm.name} returned failure exit code.")
                self.results.append(self.machine.result)
                self.send_result(self.machine.result)
                self.machine.fail()
                break
        else:
            # Check if we need to run a final execution on the previous execution + dangling set of runs
            if current_runs:
                self.machine.setup_algorithm()
                apply_iov = IoV(last_successful_result.iov.exp_low,
                                last_successful_result.iov.run_low,
                                *highest_exprun)
                B2INFO(f"Executing on {apply_iov}.")
                self.machine.execute_runs(runs=current_runs, iteration=iteration, apply_iov=apply_iov)
                B2INFO(f"Finished execution with result code {self.machine.result.result}.")
                if (self.machine.result.result == AlgResult.ok.value) or (
                        self.machine.result.result == AlgResult.iterate.value):
                    self.machine.complete()
                    # Commit all the payloads and send out the results
                    self.machine.algorithm.algorithm.commit()
                    # Save the result
                    self.results.append(self.machine.result)
                    self.send_result(self.machine.result)
                else:
                    # Save the result
                    self.results.append(self.machine.result)
                    self.send_result(self.machine.result)
                    # But failed
                    self.machine.fail()


class SimpleRunByRun(AlgorithmStrategy):
    """
    Algorithm strategy to do run-by-run calibration of collected data.
    Runs the algorithm over the input data contained within the requested IoV, starting with the first run's data only.

    This strategy differs from `SequentialRunByRun` in that it *will not merge run data* if the algorithm returns
    'not enough data' on the current run.

    Once an execution on a run returns *any* result 'iterate', 'ok', 'not_enough_data', or 'failure', we move onto the
    next run (if any are left).
    Committing of payloads to the outputdb only happens for 'iterate' or 'ok' return codes.

    .. important:: Unlike most other strategies, this one won't immediately fail and return if a run returns a 'failure' exit
                   code.
                   The failure will prevent iteration/successful completion of the CAF though.

    .. warning:: Since this strategy doesn't try to merge data from runs, if *any* run in your input data doesn't contain
                 enough data to complete the algorithm successfully, you won't be able to get a successful calibration.
                 The CAF then won't allow you to iterate this calibration, or pass the constants onward to another calibration.
                 However, you will still have the database created that covers all the successfull runs.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""

    allowed_granularities = ["run"]
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {}

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

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
        B2INFO(f"Starting AlgorithmMachine of {self.algorithm.name}.")
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO(f"Beginning execution of {self.algorithm.name} using strategy {self.__class__.__name__}.")

        all_runs_collected = set(runs_from_vector(self.algorithm.algorithm.getRunListFromAllData()))
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            B2INFO(f"Removing the ignored_runs from the runs to execute for {self.algorithm.name}.")
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)

        # Is this the first time executing the algorithm?
        first_execution = True
        for exprun in runs_to_execute:
            if not first_execution:
                self.machine.setup_algorithm()
            current_runs = exprun
            apply_iov = iov_from_runs([current_runs])
            B2INFO(f"Executing on IoV = {apply_iov}.")
            self.machine.execute_runs(runs=[current_runs], iteration=iteration, apply_iov=apply_iov)
            first_execution = False
            B2INFO(f"Finished execution with result code {self.machine.result.result}.")
            # Does this count as a successful execution?
            if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                # Commit the payloads and result
                B2INFO(f"Committing payloads for {iov_from_runs([current_runs])}.")
                self.machine.algorithm.algorithm.commit()
                self.results.append(self.machine.result)
                self.send_result(self.machine.result)
                self.machine.complete()
            # If it wasn't successful, was it due to lack of data in the runs?
            elif (self.machine.result.result == AlgResult.not_enough_data.value):
                B2INFO(f"There wasn't enough data in the IoV {iov_from_runs([current_runs])}.")
                self.results.append(self.machine.result)
                self.send_result(self.machine.result)
                self.machine.fail()
            elif self.machine.result.result == AlgResult.failure.value:
                B2ERROR(f"Failure exit code in the IoV {iov_from_runs([current_runs])}.")
                self.results.append(self.machine.result)
                self.send_result(self.machine.result)
                self.machine.fail()

        # Print any knowable gaps between result IoVs, if any are foun there is a problem.
        gaps = self.find_iov_gaps()
        # Dump them to a file for logging
        with open(f"{self.algorithm.name}_iov_gaps.json", "w") as f:
            json.dump(gaps, f)

        self.send_final_state(self.COMPLETED)


class SequentialBoundaries(AlgorithmStrategy):
    """
    Algorithm strategy to first calculate run boundaries where execution should be attempted.
    Runs the algorithm over the input data contained within the requested IoV of the boundaries,
    starting with the first boundary data only.
    If the algorithm returns 'not enough data' on the current boundary IoV, it won't commit the payloads,
    but instead adds the next boundarie's data and tries again. Basically the same logic as `SequentialRunByRun`
    but using run boundaries instead of runs directly.
    Notice that boundaries cannot span multiple experiments.

    By default the algorithm will get the payload boundaries directly from the algorithm that need to
    have inplemented the function ``isBoundaryRequired``. If the desired boundaries are already known it
    is possible to pass them directly setting the algorithm parameter ``payload_boundaries`` and avoid
    the need to define the  ``isBoundaryRequired`` function.

    ``payload_boundaries`` is a list ``[(exp1, run1), (exp2, run2), ...]``. A boundary at the beginning of each
    experiment will be added if not already present. An empty list will thus produce a single payload for each
    experiment. A ``payload_boundaries`` set to ``None`` is equivalent to not passing it and restores the default
    behaviour where the boundaries are computed in the ``isBoundaryRequired`` function of the algorithm.
    """
    #: The params that you could set on the Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {
        "iov_coverage": IoV,
        "payload_boundaries": []  # [(exp1, run1), (exp2, run2), ...]
    }

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)
        self.first_execution = True

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

        # Now iterate through the experiments. We DO NOT allow a payload IoV to
        # extend over multiple experiments, only multiple runs
        iov_coverage = None
        if "iov_coverage" in self.algorithm.params:
            B2INFO(f"Detected that you have set iov_coverage to {self.algorithm.params['iov_coverage']}.")
            iov_coverage = self.algorithm.params["iov_coverage"]

        payload_boundaries = None
        if "payload_boundaries" in self.algorithm.params:
            B2INFO(f"Detected that you have set payload_boundaries to {self.algorithm.params['payload_boundaries']}.")
            payload_boundaries = self.algorithm.params["payload_boundaries"]

        number_of_experiments = len(runs_to_execute)
        B2INFO(f"We are iterating over {number_of_experiments} experiments.")

        # Iterate over experiment run lists
        for i_exp, run_list in enumerate(runs_to_execute, start=1):
            B2DEBUG(26, f"Run List for this experiment={run_list}")
            current_experiment = run_list[0].exp
            B2INFO(f"Executing over data from experiment {current_experiment}")
            # If 'iov_coverage' was set in the algorithm.params and it is larger (at both ends) than the
            # input data runs IoV, then we also have to set the first payload IoV to encompass the missing beginning
            # of the iov_coverage, and the last payload IoV must cover up to the end of iov_coverage.
            # This is only true for the lowest and highest experiments in our input data.
            if i_exp == 1:
                if iov_coverage:
                    lowest_exprun = ExpRun(iov_coverage.exp_low, iov_coverage.run_low)
                else:
                    lowest_exprun = run_list[0]
            # We are calibrating across multiple experiments so we shouldn't start from the middle but from the 0th run
            else:
                lowest_exprun = ExpRun(current_experiment, 0)

            # Override the normal value for the highest ExpRun (from data) if iov_coverage was set
            if iov_coverage and i_exp == number_of_experiments:
                highest_exprun = ExpRun(iov_coverage.exp_high, iov_coverage.run_high)
            # If we have more experiments to execute then we wil be setting the final payload IoV in this experiment
            # to be unbounded
            elif i_exp < number_of_experiments:
                highest_exprun = ExpRun(current_experiment, -1)
            # Otherwise just get the values from data
            else:
                highest_exprun = run_list[-1]

            # Find the boundaries for this experiment's runs
            vec_run_list = vector_from_runs(run_list)
            if payload_boundaries is None:
                # Find the boundaries using the findPayloadBoundaries implemented in the algorithm
                B2INFO("Attempting to find payload boundaries.")
                vec_boundaries = self.algorithm.algorithm.findPayloadBoundaries(vec_run_list)
                # If this vector is empty then that's bad. Maybe the isBoundaryRequired function
                # wasn't implemented? Either way we should stop.
                if vec_boundaries.empty():
                    B2ERROR("No boundaries found but we are in a strategy that requires them! Failing...")
                    # Tell the Runner that we have failed
                    self.send_final_state(self.FAILED)
                    break
                vec_boundaries = runs_from_vector(vec_boundaries)
            else:
                # Using boundaries set by user
                B2INFO(f"Using as payload boundaries {payload_boundaries}.")
                vec_boundaries = [ExpRun(exp, run) for exp, run in payload_boundaries]
                # No need to check that vec_boundaries is not empty. In case it is we will anyway add
                # a boundary at the first run of each experiment.
            # Remove any boundaries not from the current experiment (only likely if they were set manually)
            # We sort just to make everything easier later and just in case something mad happened.
            run_boundaries = sorted([er for er in vec_boundaries if er.exp == current_experiment])
            # In this strategy we consider separately each experiment. We then now check that the
            # boundary (exp, 0) is present and if not we add it. It is indeed possible to miss it
            # if the boundaries were given manually
            first_exprun = ExpRun(current_experiment, 0)
            if first_exprun not in run_boundaries:
                B2WARNING(f"No boundary found at ({current_experiment}, 0), adding it.")
                run_boundaries[0:0] = [first_exprun]
            B2INFO((f"Found {len(run_boundaries)} boundaries for this experiment. "
                    "Checking if we have some data for all boundary IoVs..."))
            # First figure out the run lists to use for each execution (potentially different from the applied IoVs)
            # We use the boundaries and the run_list
            boundary_iovs_to_run_lists = find_run_lists_from_boundaries(run_boundaries, run_list)
            B2DEBUG(26, f"Boundary IoVs before checking data = {boundary_iovs_to_run_lists}")
            # If there were any boundary IoVs with no run data, just remove them. Otherwise they will execute over all data.
            boundary_iovs_to_run_lists = {key: value for key, value in boundary_iovs_to_run_lists.items() if value}
            B2DEBUG(26, f"Boundary IoVs after checking data = {boundary_iovs_to_run_lists}")
            # If any were removed then we might have gaps between the boundary IoVs. Fix those now by merging IoVs.
            new_boundary_iovs_to_run_lists = {}
            previous_boundary_iov = None
            previous_boundary_run_list = None
            for boundary_iov, run_list in boundary_iovs_to_run_lists.items():
                if not previous_boundary_iov:
                    previous_boundary_iov = boundary_iov
                    previous_boundary_run_list = run_list
                    continue
                # We are definitely dealiing with IoVs from one experiment so we can make assumptions here
                if previous_boundary_iov.run_high != (boundary_iov.run_low-1):
                    B2WARNING("Gap in boundary IoVs found before execution! "
                              "Will correct it by extending the previous boundary up to the next one.")
                    B2INFO(f"Original boundary IoV={previous_boundary_iov}")
                    previous_boundary_iov = IoV(previous_boundary_iov.exp_low, previous_boundary_iov.run_low,
                                                previous_boundary_iov.exp_high, boundary_iov.run_low-1)
                    B2INFO(f"New boundary IoV={previous_boundary_iov}")
                new_boundary_iovs_to_run_lists[previous_boundary_iov] = previous_boundary_run_list
                previous_boundary_iov = boundary_iov
                previous_boundary_run_list = run_list
            else:
                new_boundary_iovs_to_run_lists[previous_boundary_iov] = previous_boundary_run_list
            boundary_iovs_to_run_lists = new_boundary_iovs_to_run_lists
            B2DEBUG(26, f"Boundary IoVs after fixing gaps = {boundary_iovs_to_run_lists}")
            # Actually execute now that we have an IoV list to apply
            success = self.execute_over_boundaries(boundary_iovs_to_run_lists, lowest_exprun, highest_exprun, iteration)
            if not success:
                # Tell the Runner that we have failed
                self.send_final_state(self.FAILED)
                break
        # Only executes if we didn't fail any experiment execution
        else:
            # Print any knowable gaps between result IoVs, if any are found there is a problem, but not necessarily too bad.
            gaps = self.find_iov_gaps()
            if gaps:
                B2WARNING("There were gaps between the output IoV payloads! See the JSON file in the algorithm output directory.")
            # Dump them to a file for logging
            with open(f"{self.algorithm.name}_iov_gaps.json", "w") as f:
                json.dump(gaps, f)

            # If any results weren't successes we fail
            if self.any_failed_iov():
                self.send_final_state(self.FAILED)
            else:
                self.send_final_state(self.COMPLETED)

    def execute_over_boundaries(self, boundary_iovs_to_run_lists, lowest_exprun, highest_exprun, iteration):
        """
        Take the previously found boundaries and the run lists they correspond to and actually perform the
        Algorithm execution. This is assumed to be for a single experiment.
        """
        # Copy of boundary IoVs
        remaining_boundary_iovs = sorted(list(boundary_iovs_to_run_lists.keys())[:])

        # The current runs we are executing
        current_runs = []
        # The IoV of the current boundary(s)
        current_boundary_iov = None
        # The current execution's applied IoV, may be different to the boundary IoV
        current_iov = None

        # The last successful payload list and result. We hold on to them so that we can commit or discard later.
        last_successful_payloads = None
        last_successful_result = None
        # The previous execution's runs
        last_successful_runs = []
        # The previous execution's applied IoV
        last_successful_iov = None

        while True:
            # Do we have previous successes?
            if not last_successful_result:
                if not current_runs:
                    # Did we actually have any boundaries?
                    if not remaining_boundary_iovs:
                        # Fail because we have no boundaries to use
                        B2ERROR("No boundaries found for the current experiment's run list. Failing the strategy.")
                        return False

                    B2INFO("This appears to be the first attempted execution of the experiment.")
                    # Attempt to execute on the first boundary
                    current_boundary_iov = remaining_boundary_iovs.pop(0)
                    current_runs = boundary_iovs_to_run_lists[current_boundary_iov]
                    # What if there is only one boundary? Need to apply the highest exprun
                    if not remaining_boundary_iovs:
                        current_iov = IoV(*lowest_exprun, *highest_exprun)
                    else:
                        current_iov = IoV(*lowest_exprun, current_boundary_iov.exp_high, current_boundary_iov.run_high)
                # Returned not enough data from first execution
                else:
                    # Any remaining boundaries?
                    if not remaining_boundary_iovs:
                        # Fail because we have no boundaries to use
                        B2ERROR("Not enough data found for the current experiment's run list. Failing the strategy.")
                        return False

                    B2INFO("There wasn't enough data previously. Merging with the runs from the next boundary.")
                    # Extend the previous run lists/iovs
                    next_boundary_iov = remaining_boundary_iovs.pop(0)
                    current_boundary_iov = IoV(current_boundary_iov.exp_low, current_boundary_iov.run_low,
                                               next_boundary_iov.exp_high, next_boundary_iov.run_high)
                    current_runs.extend(boundary_iovs_to_run_lists[next_boundary_iov])
                    # At the last boundary? Need to apply the highest exprun
                    if not remaining_boundary_iovs:
                        current_iov = IoV(current_iov.exp_low, current_iov.run_low, *highest_exprun)
                    else:
                        current_iov = IoV(current_iov.exp_low, current_iov.run_low,
                                          current_boundary_iov.exp_high, current_boundary_iov.run_high)

                self.execute_runs(current_runs, iteration, current_iov)

                # Does this count as a successful execution?
                if self.alg_success():
                    # Commit previous values we were holding onto
                    B2INFO("Found a success. Will save the payloads for later.")
                    # Save success
                    last_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                    last_successful_result = self.machine.result
                    last_successful_runs = current_runs[:]
                    last_successful_iov = current_iov
                    # Reset values for next loop
                    current_runs = []
                    current_boundary_iov = None
                    current_iov = None
                    self.machine.complete()
                    continue
                elif self.machine.result.result == AlgResult.not_enough_data.value:
                    B2INFO("Not Enough Data result.")
                    # Just complete but leave the current runs alone for next loop
                    self.machine.complete()
                    continue
                else:
                    B2ERROR("Hit a failure or some kind of result we can't continue from. Failing out...")
                    self.machine.fail()
                    return False
            # Previous result exists
            else:
                # Previous loop was a success
                if not current_runs:
                    # Remaining boundaries?
                    if not remaining_boundary_iovs:
                        # Out of data, can now commit
                        B2INFO("Finished this experiment's boundaries. "
                               f"Committing remaining payloads from {last_successful_result.iov}")
                        self.machine.algorithm.algorithm.commit(last_successful_payloads)
                        self.results.append(last_successful_result)
                        self.send_result(last_successful_result)
                        return True

                    # Remaining boundaries exist so we try to execute
                    current_boundary_iov = remaining_boundary_iovs.pop(0)
                    current_runs = boundary_iovs_to_run_lists[current_boundary_iov]
                    # What if there is only one boundary? Need to apply the highest exprun
                    if not remaining_boundary_iovs:
                        current_iov = IoV(current_boundary_iov.exp_low, current_boundary_iov.run_low, *highest_exprun)
                    else:
                        current_iov = current_boundary_iov

                # Returned not enough data from last execution
                else:
                    # Any remaining boundaries?
                    if not remaining_boundary_iovs:
                        B2INFO("We have no remaining runs to increase the amount of data. "
                               "Instead we will merge with the previous successful runs.")
                        # Merge with previous success IoV
                        new_current_runs = last_successful_runs[:]
                        new_current_runs.extend(current_runs)
                        current_runs = new_current_runs[:]
                        current_iov = IoV(last_successful_iov.exp_low, last_successful_iov.run_low,
                                          current_iov.exp_high, current_iov.run_high)
                        # We reset the last successful stuff because we are dropping it
                        last_successful_payloads = []
                        last_successful_result = None
                        last_successful_runs = []
                        last_successful_iov = None

                    else:
                        B2INFO("Since there wasn't enough data previously, we will merge with the runs from the next boundary.")
                        # Extend the previous run lists/iovs
                        next_boundary_iov = remaining_boundary_iovs.pop(0)
                        current_boundary_iov = IoV(current_boundary_iov.exp_low, current_boundary_iov.run_low,
                                                   next_boundary_iov.exp_high, next_boundary_iov.run_high)
                        # Extend previous execution's runs with the next set
                        current_runs.extend(boundary_iovs_to_run_lists[next_boundary_iov])
                        # At the last boundary? Need to apply the highest exprun
                        if not remaining_boundary_iovs:
                            current_iov = IoV(current_iov.exp_low, current_iov.run_low, *highest_exprun)
                        else:
                            current_iov = IoV(current_iov.exp_low, current_iov.run_low,
                                              current_boundary_iov.exp_high, current_boundary_iov.run_high)

                self.execute_runs(current_runs, iteration, current_iov)

                # Does this count as a successful execution?
                if self.alg_success():
                    # Commit previous values we were holding onto
                    B2INFO("Found a success.")
                    if last_successful_result:
                        B2INFO("Can now commit the previous success.")
                        self.machine.algorithm.algorithm.commit(last_successful_payloads)
                        self.results.append(last_successful_result)
                        self.send_result(last_successful_result)
                    # Replace last success
                    last_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                    last_successful_result = self.machine.result
                    last_successful_runs = current_runs[:]
                    last_successful_iov = current_iov
                    # Reset values for next loop
                    current_runs = []
                    current_boundary_iov = None
                    current_iov = None
                    self.machine.complete()
                    continue
                elif self.machine.result.result == AlgResult.not_enough_data.value:
                    B2INFO("Not Enough Data result.")
                    # Just complete but leave the current runs alone for next loop
                    self.machine.complete()
                    continue
                else:
                    B2ERROR("Hit a failure or some other result we can't continue from. Failing out...")
                    self.machine.fail()
                    return False

    def execute_runs(self, runs, iteration, iov):
        # Already set up earlier the first time, so we shouldn't do it again
        if not self.first_execution:
            self.machine.setup_algorithm()
        else:
            self.first_execution = False

        B2INFO(f"Executing and applying {iov} to the payloads.")
        self.machine.execute_runs(runs=runs, iteration=iteration, apply_iov=iov)
        B2INFO(f"Finished execution with result code {self.machine.result.result}.")

    def alg_success(self):
        return ((self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value))


class StrategyError(Exception):
    """
    Basic Exception for this type of class.
    """
