# -*- coding: utf-8 -*-

"""Custom calibration strategy for KLM channel status."""

import collections
import os

import basf2
from ROOT import Belle2

from caf.utils import ExpRun, IoV, AlgResult
from caf.utils import runs_overlapping_iov, runs_from_vector
from caf.utils import split_runs_by_exp
from caf.strategies import AlgorithmStrategy
from caf.state_machines import AlgorithmMachine
from ROOT.Belle2 import KLMChannelStatusAlgorithm
from klm_strategies_common import get_lowest_exprun, get_highest_exprun, \
                                  calibration_result_string


class KLMChannelStatus(AlgorithmStrategy):
    """
    Custom strategy for executing the KLM channel status. Requires complex
    run merging rules.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute
    the various steps rather than operating on a CalibrationAlgorithm
    C++ class directly.
    """

    #: The parameters of Algorithm object which this Strategy would use.
    #: Just here for documentation reasons.
    usable_params = {'iov_coverage': IoV}

    def __init__(self, algorithm):
        """
        """
        super().__init__(algorithm)
        #: :py:class:`caf.state_machines.AlgorithmMachine` used to help
        #: set up and execute CalibrationAlgorithm. It gets set up properly
        #: in :py:func:`run`.
        self.machine = AlgorithmMachine(self.algorithm)
        #: Flag for the first execution of this AlgorithmStrategy
        self.first_execution = True

    def run(self, iov, iteration, queue):
        """
        Runs the algorithm machine over the collected data and
        fills the results.
        """
        if not self.is_valid():
            raise StrategyError('The strategy KLMChannelStatus was not '
                                'set up correctly.')
        #: The multiprocessing queue used to pass back results one at a time.
        self.queue = queue

        basf2.B2INFO(f'Setting up {self.__class__.__name__} strategy '
                     f'for {self.algorithm.name}')
        # Add all the necessary parameters for a strategy to run.
        machine_params = {}
        machine_params['database_chain'] = self.database_chain
        machine_params['dependent_databases'] = self.dependent_databases
        machine_params['output_dir'] = self.output_dir
        machine_params['output_database_dir'] = self.output_database_dir
        machine_params['input_files'] = self.input_files
        machine_params['ignored_runs'] = self.ignored_runs
        self.machine.setup_from_dict(machine_params)
        # Start moving through machine states.
        basf2.B2INFO(f'Starting AlgorithmMachine of {self.algorithm.name}')
        # This sets up the logging and database chain and assigns all
        # input files from collector jobs.
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm.
        basf2.B2INFO(f'Beginning execution of {self.algorithm.name} using '
                     f'strategy {self.__class__.__name__}')

        # Select of runs for calibration.
        runs = self.algorithm.algorithm.getRunListFromAllData()
        all_runs_collected = set(runs_from_vector(runs))
        # Select runs overlapping with the calibration IOV if it is specified.
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected
        # Remove the ignored runs.
        if self.ignored_runs:
            basf2.B2INFO(f'Removing the ignored_runs from the runs '
                         f'to execute for {self.algorithm.name}')
            runs_to_execute.difference_update(set(self.ignored_runs))

        # Creation of sorted run list split by experiment.
        runs_to_execute = sorted(runs_to_execute)
        runs_to_execute = split_runs_by_exp(runs_to_execute)

        # Get IOV coverage,
        iov_coverage = None
        if 'iov_coverage' in self.algorithm.params:
            iov_coverage = self.algorithm.params['iov_coverage']

        # Iterate over experiment run lists.
        number_of_experiments = len(runs_to_execute)
        for i_exp, run_list in enumerate(runs_to_execute, start=1):
            lowest_exprun = get_lowest_exprun(number_of_experiments, i_exp,
                                              run_list, iov_coverage)
            highest_exprun = get_highest_exprun(number_of_experiments, i_exp,
                                                run_list, iov_coverage)
            self.process_experiment(run_list[0].exp, run_list, iteration,
                                    lowest_exprun, highest_exprun)

        # Send final state and result to CAF.
        self.send_result(self.machine.result)
        if (self.machine.result.result == AlgResult.ok.value) or \
           (self.machine.result.result == AlgResult.iterate.value):
            self.send_final_state(self.COMPLETED)
        else:
            self.send_final_state(self.FAILED)

    def execute_over_run_list(self, run_list, iteration, forced_calibration):
        """
        Execute over run list.
        """
        if not self.first_execution:
            self.machine.setup_algorithm()
        else:
            self.first_execution = False
        self.machine.algorithm.algorithm.setForcedCalibration(
            forced_calibration)
        self.machine.execute_runs(runs=run_list, iteration=iteration,
                                  apply_iov=None)
        if (self.machine.result.result == AlgResult.ok.value) or \
           (self.machine.result.result == AlgResult.iterate.value):
            self.machine.complete()
        else:
            self.machine.fail()

    def process_experiment(self, experiment, experiment_runs, iteration,
                           lowest_exprun, highest_exprun):
        """
        Process runs from experiment.
        """
        # Run lists. They have the following format: run number,
        # calibration result code, ExpRun, algorithm results,
        # merge information, payload.
        run_data = []
        run_data_klm_excluded = []

        # Initial run.
        for exp_run in experiment_runs:
            self.execute_over_run_list([exp_run], iteration, False)
            result = self.machine.result.result
            algorithm_results = KLMChannelStatusAlgorithm.Results(
                self.machine.algorithm.algorithm.getResults())
            if (algorithm_results.getTotalHitNumber() > 0):
                run_data.append([exp_run.run, result, [exp_run],
                                 algorithm_results, '', None])
            else:
                run_data_klm_excluded.append([exp_run.run, result, [exp_run],
                                              algorithm_results, '', None])
            result_str = calibration_result_string(result)
            basf2.B2INFO('Run %d: %s.' % (exp_run.run, result_str))
