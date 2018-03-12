#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import B2ERROR, B2FATAL, B2INFO
from .utils import AlgResult
from .utils import runs_overlapping_iov
from .utils import runs_from_vector
from .utils import iov_from_runs
from .state_machines import AlgorithmMachine

from abc import ABC, abstractmethod


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
                      "global_tag",
                      "local_database_chain",
                      "dependent_databases",
                      "output_dir",
                      "output_database_dir",
                      "input_files"
                      ]

    #: Attributes that must have a value that returns True when tested by :py:meth:`is_valid`.
    required_true_attrs = ["algorithm",
                           "output_dir",
                           "output_database_dir",
                           "input_files"
                           ]

    #: Granularity of collector that can be run by this algorithm properly
    allowed_granularities = ["run", "all"]

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
                B2ERROR("AlgorithmStrategy attribute {} doesn't exist.".format(attribute_name))
                return False
        # Check if any attributes that need actual values haven't been set or were empty
        for attribute_name in self.required_true_attrs:
            if not getattr(self, attribute_name):
                B2ERROR("AlgorithmStrategy attribute {} returned False.".format(attribute_name))
                return False
        return True


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
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

    def run(self, iov, iteration):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["global_tag"] = self.global_tag
        machine_params["local_database_chain"] = self.local_database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))
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
            # Valid exit codes mean we can complete properly
            self.machine.complete()
            # Commit all the payloads and send out the results
            self.machine.algorithm.algorithm.commit()
        else:
            # Either there wasn't enough data or the algorithm failed
            self.machine.fail()


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

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly.
"""

    allowed_granularities = ["run"]

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

    def run(self, iov, iteration):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["global_tag"] = self.global_tag
        machine_params["local_database_chain"] = self.local_database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))
        runs_to_execute = []
        all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
        # If we were given a specific IoV to calibrate we just execute over runs in that IoV
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected[:]
        # The runs we have left to execute
        remaining_runs = runs_to_execute[:]
        # The previous execution's runs
        previous_runs = []
        # The current runs we are executing
        current_runs = []
        # The last successful payload and result
        last_successful_payloads = None
        last_successful_result = None
        # Is this the first time executing the algorithm?
        first_execution = True
        for exprun in runs_to_execute:
            if not first_execution:
                self.machine.setup_algorithm()
            # Add on the next run
            current_runs.append(exprun)
            # Remove it from our remaining runs
            remaining_runs.pop(0)
            B2INFO("Executing on IoV = {}".format(iov_from_runs(current_runs)))
            self.machine.execute_runs(runs=current_runs, iteration=iteration)
            first_execution = False
            B2INFO("Finished execution with result code {}".format(self.machine.result.result))
            # Does this count as a successful execution?
            if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                self.machine.complete()
                # If we've succeeded but we have a previous success we can commit the previous payloads
                # since we have new ones ready
                if last_successful_payloads and last_successful_result:
                    B2INFO("Saving the most recent payloads for {} to be committed later.".format(iov_from_runs(current_runs)))
                    # Save the payloads and result
                    new_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                    new_successful_result = self.machine.result
                    B2INFO("We just succeded in execution of the Algorithm."
                           " Will now commit payloads from the previous success for {}.".format(last_successful_result.iov))
                    self.machine.algorithm.algorithm.commit(last_successful_payloads)
                    self.results.append(last_successful_result)
                    # If there are remaining runs we need to have the current payloads ready to commit after the next execution
                    if remaining_runs:
                        last_successful_payloads = new_successful_payloads
                        last_successful_result = new_successful_result
                    # If there's not more runs to process we should also commit the new ones
                    else:
                        B2INFO("We have no more runs to process. "
                               "Will now commit the most recent payloads for {}".format(new_successful_result.iov))
                        self.machine.algorithm.algorithm.commit(new_successful_payloads)
                        self.results.append(new_successful_result)
                        break
                # if there's no previous success this must be the first run executed
                else:
                    # Need to save payloads for later if we have a success but runs remain
                    if remaining_runs:
                        B2INFO("Saving the most recent payloads for {} to be committed later.".format(iov_from_runs(current_runs)))
                        # Save the payloads and result
                        last_successful_payloads = self.machine.algorithm.algorithm.getPayloadValues()
                        last_successful_result = self.machine.result
                    # Need to commit and exit if we have a success and no remaining data
                    else:
                        B2INFO("We just succeeded in execution of the Algorithm."
                               " No runs left to be processed, so we are committing results of this execution.")
                        self.machine.algorithm.algorithm.commit()
                        self.results.append(self.machine.result)
                        break

                previous_runs = current_runs[:]
                current_runs = []
            # If it wasn't successful, was it due to lack of data in the runs?
            elif (self.machine.result.result == AlgResult.not_enough_data.value):
                B2INFO("There wasn't enough data in the IoV {}".format(iov_from_runs(current_runs)))
                if remaining_runs:
                    B2INFO("Some runs remain to be processed will try to merge the IoV with them")
                elif not remaining_runs and not last_successful_result:
                    B2ERROR("There aren't any more runs remaining to merge with, and we never had a previous success."
                            " There wasn't enough data in the full input data requested.")
                    self.results.append(self.machine.result)
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
                B2ERROR("{} returned failure exit code.".format(self.algorithm.name))
                self.results.append(self.machine.result)
                self.machine.fail()
                break
        else:
            # Check if we need to run a final execution on the previous execution + dangling set of runs
            if current_runs:
                self.machine.setup_algorithm()
                B2INFO("Executing on IoV = {}".format(iov_from_runs(current_runs)))
                self.machine.execute_runs(runs=current_runs, iteration=iteration)
                B2INFO("Finished execution with result code {}".format(self.machine.result.result))
                if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                    self.machine.complete()
                    # Commit all the payloads and send out the results
                    self.machine.algorithm.algorithm.commit()
                    # Save the result
                    self.results.append(self.machine.result)
                else:
                    # Save the result
                    self.results.append(self.machine.result)
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

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help set up and execute CalibrationAlgorithm
        #: It gets setup properly in :py:func:`run`
        self.machine = AlgorithmMachine(self.algorithm)

    def run(self, iov, iteration):
        """
        Runs the algorithm machine over the collected data and fills the results.
        """
        if not self.is_valid():
            raise StrategyError("This AlgorithmStrategy was not set up correctly!")
        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
        # Now add all the necessary parameters for a strategy to run
        machine_params = {}
        machine_params["global_tag"] = self.global_tag
        machine_params["local_database_chain"] = self.local_database_chain
        machine_params["dependent_databases"] = self.dependent_databases
        machine_params["output_dir"] = self.output_dir
        machine_params["output_database_dir"] = self.output_database_dir
        machine_params["input_files"] = self.input_files
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))
        runs_to_execute = []
        all_runs_collected = runs_from_vector(self.algorithm.algorithm.getRunListFromAllData())
        # If we were given a specific IoV to calibrate we just execute over runs in that IoV
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected[:]
        # The runs we have left to execute
        remaining_runs = runs_to_execute[:]
        # Is this the first time executing the algorithm?
        first_execution = True
        for exprun in runs_to_execute:
            if not first_execution:
                self.machine.setup_algorithm()
            current_runs = exprun
            # Remove current run from our remaining runs
            remaining_runs.pop(0)
            B2INFO("Executing on IoV = {}".format(iov_from_runs([current_runs])))
            self.machine.execute_runs(runs=[current_runs], iteration=iteration)
            first_execution = False
            B2INFO("Finished execution with result code {}".format(self.machine.result.result))
            # Does this count as a successful execution?
            if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
                # Commit the payloads and result
                B2INFO("Committing payloads for {}.".format(iov_from_runs([current_runs])))
                self.machine.algorithm.algorithm.commit()
                self.results.append(self.machine.result)
                self.machine.complete()
            # If it wasn't successful, was it due to lack of data in the runs?
            elif (self.machine.result.result == AlgResult.not_enough_data.value):
                B2INFO("There wasn't enough data in the IoV {}".format(iov_from_runs([current_runs])))
                self.results.append(self.machine.result)
                self.machine.fail()
            elif self.machine.result.result == AlgResult.failure.value:
                B2ERROR("Failure exit code in the IoV {}".format(iov_from_runs([current_runs])))
                self.results.append(self.machine.result)
                self.machine.fail()


class StrategyError(Exception):
    """
    Basic Exception for this type of class
    """
    pass
