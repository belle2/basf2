##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# -*- coding: utf-8 -*-

"""Custom calibration strategy for KLM channel status."""

import numpy

import basf2
import ROOT
from ROOT import Belle2

from caf.utils import AlgResult, IoV
from caf.utils import runs_overlapping_iov, runs_from_vector
from caf.utils import split_runs_by_exp
from caf.strategies import AlgorithmStrategy, StrategyError
from caf.state_machines import AlgorithmMachine
from ROOT.Belle2 import KLMChannelStatusAlgorithm, KLMChannelIndex
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
            payload = self.machine.algorithm.algorithm.getPayloadValues()
            run_results = [
                exp_run.run, result, [exp_run], algorithm_results, '', payload]
            if (algorithm_results.getTotalHitNumber() > 0):
                run_data.append(run_results)
            else:
                run_data_klm_excluded.append(run_results)
            result_str = calibration_result_string(result)
            basf2.B2INFO('Run %d: %s.' % (exp_run.run, result_str))

        # Sort by run number.
        run_data.sort(key=lambda x: x[0])
        run_data_klm_excluded.sort(key=lambda x: x[0])

        # Create a tree with number of events.
        save_channel_hit_map = False
        save_module_hit_map = True
        save_sector_hit_map = True
        f_hit_map = ROOT.TFile('hit_map.root', 'recreate')
        run = numpy.zeros(1, dtype=int)
        calibration_result = numpy.zeros(1, dtype=int)
        module = numpy.zeros(1, dtype=int)
        subdetector = numpy.zeros(1, dtype=int)
        section = numpy.zeros(1, dtype=int)
        sector = numpy.zeros(1, dtype=int)
        layer = numpy.zeros(1, dtype=int)
        plane = numpy.zeros(1, dtype=int)
        strip = numpy.zeros(1, dtype=int)
        hits_total = numpy.zeros(1, dtype=int)
        hits_module = numpy.zeros(1, dtype=int)
        active_channels = numpy.zeros(1, dtype=int)
        hit_map_channel = ROOT.TTree('hit_map_channel', '')
        hit_map_channel.Branch('run', run, 'run/I')
        hit_map_channel.Branch('calibration_result', calibration_result,
                               'calibration_result/I')
        hit_map_channel.Branch('channel', module, 'channel/I')
        hit_map_channel.Branch('subdetector', subdetector, 'subdetector/I')
        hit_map_channel.Branch('section', section, 'section/I')
        hit_map_channel.Branch('sector', sector, 'sector/I')
        hit_map_channel.Branch('layer', layer, 'layer/I')
        hit_map_channel.Branch('plane', plane, 'plane/I')
        hit_map_channel.Branch('strip', strip, 'strip/I')
        hit_map_channel.Branch('hits_total', hits_total, 'hits_total/I')
        hit_map_channel.Branch('hits_channel', hits_module, 'hits_channel/I')
        hit_map_module = ROOT.TTree('hit_map_module', '')
        hit_map_module.Branch('run', run, 'run/I')
        hit_map_module.Branch('calibration_result', calibration_result,
                              'calibration_result/I')
        hit_map_module.Branch('module', module, 'module/I')
        hit_map_module.Branch('subdetector', subdetector, 'subdetector/I')
        hit_map_module.Branch('section', section, 'section/I')
        hit_map_module.Branch('sector', sector, 'sector/I')
        hit_map_module.Branch('layer', layer, 'layer/I')
        hit_map_module.Branch('hits_total', hits_total, 'hits_total/I')
        hit_map_module.Branch('hits_module', hits_module, 'hits_module/I')
        hit_map_module.Branch('active_channels', active_channels,
                              'active_channels/I')
        hit_map_sector = ROOT.TTree('hit_map_sector', '')
        hit_map_sector.Branch('run', run, 'run/I')
        hit_map_sector.Branch('calibration_result', calibration_result,
                              'calibration_result/I')
        hit_map_sector.Branch('sector_global', module, 'sector_global/I')
        hit_map_sector.Branch('subdetector', subdetector, 'subdetector/I')
        hit_map_sector.Branch('section', section, 'section/I')
        hit_map_sector.Branch('sector', sector, 'sector/I')
        hit_map_sector.Branch('hits_total', hits_total, 'hits_total/I')
        hit_map_sector.Branch('hits_sector', hits_module, 'hits_sector/I')
        for i in range(0, len(run_data)):
            run[0] = run_data[i][0]
            calibration_result[0] = run_data[i][1]
            hits_total[0] = run_data[i][3].getTotalHitNumber()
            # Channel hit map.
            if (save_channel_hit_map):
                index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelStrip)
                index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelStrip)
                while (index != index2.end()):
                    module[0] = index.getKLMChannelNumber()
                    subdetector[0] = index.getSubdetector()
                    section[0] = index.getSection()
                    sector[0] = index.getSector()
                    layer[0] = index.getLayer()
                    plane[0] = index.getPlane()
                    strip[0] = index.getStrip()
                    hits_module[0] = run_data[i][3].getHitMapChannel(). \
                        getChannelData(int(module[0]))
                    hit_map_channel.Fill()
                    index.increment()
            # Module hit map.
            if (save_module_hit_map):
                index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
                index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelLayer)
                while (index != index2.end()):
                    module[0] = index.getKLMModuleNumber()
                    subdetector[0] = index.getSubdetector()
                    section[0] = index.getSection()
                    sector[0] = index.getSector()
                    layer[0] = index.getLayer()
                    hits_module[0] = run_data[i][3].getHitMapModule(). \
                        getChannelData(int(module[0]))
                    active_channels[0] = run_data[i][3]. \
                        getModuleActiveChannelMap(). \
                        getChannelData(int(module[0]))
                    hit_map_module.Fill()
                    index.increment()
            # Sector hit map.
            if (save_sector_hit_map):
                index = KLMChannelIndex(KLMChannelIndex.c_IndexLevelSector)
                index2 = KLMChannelIndex(KLMChannelIndex.c_IndexLevelSector)
                while (index != index2.end()):
                    module[0] = index.getKLMSectorNumber()
                    subdetector[0] = index.getSubdetector()
                    section[0] = index.getSection()
                    sector[0] = index.getSector()
                    hits_module[0] = run_data[i][3].getHitMapSector(). \
                        getChannelData(int(module[0]))
                    hit_map_sector.Fill()
                    index.increment()
        hit_map_channel.Write()
        hit_map_module.Write()
        hit_map_sector.Write()
        f_hit_map.Close()

        # Create list of runs that do not have enough data.
        run_ranges = []
        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == 2):
                j = i
                while (run_data[j][1] == 2):
                    j += 1
                    if (j >= len(run_data)):
                        break
                run_ranges.append([i, j])
                i = j
            else:
                i += 1

        # Determine whether the runs with insufficient data can be merged
        # to the next or previous normal run.
        def can_merge(run_data, run_not_enough_data, run_normal):
            return run_data[run_not_enough_data][3].getModuleStatus(). \
                newNormalChannels(
                run_data[run_normal][3].getModuleStatus()) == 0

        for run_range in run_ranges:
            next_run = run_range[1]
            # To mark as 'none' at the end if there are no normal runs.
            j = run_range[0]
            i = next_run - 1
            if (next_run < len(run_data)):
                while (i >= run_range[0]):
                    if (can_merge(run_data, i, next_run)):
                        basf2.B2INFO(
                            'Run %d (not enough data) can be merged into '
                            'the next normal run %d.' %
                            (run_data[i][0], run_data[next_run][0]))
                        run_data[i][4] = 'next'
                    else:
                        basf2.B2INFO(
                            'Run %d (not enough data) cannot be merged into '
                            'the next normal run %d, will try the previous '
                            'one.' % (run_data[i][0], run_data[next_run][0]))
                        break
                    i -= 1
                if (i < run_range[0]):
                    continue
            previous_run = run_range[0] - 1
            if (previous_run >= 0):
                while (j <= i):
                    if (can_merge(run_data, j, previous_run)):
                        basf2.B2INFO(
                            'Run %d (not enough data) can be merged into '
                            'the previous normal run %d.' %
                            (run_data[j][0], run_data[previous_run][0]))
                        run_data[j][4] = 'previous'
                    else:
                        basf2.B2INFO(
                            'Run %d (not enough data) cannot be merged into '
                            'the previous normal run %d.' %
                            (run_data[j][0], run_data[previous_run][0]))
                        break
                    j += 1
                if (j > i):
                    continue
            basf2.B2INFO('A range of runs with not enough data is found '
                         'that cannot be merged into neither previous nor '
                         'next normal run: from %d to %d.' %
                         (run_data[j][0], run_data[i][0]))
            while (j <= i):
                run_data[j][4] = 'none'
                j += 1

        # Merge runs that do not have enough data. If both this and next
        # run do not have enough data, then merge the collected data.
        i = 0
        j = 0
        while (i < len(run_data) - 1):
            while ((run_data[i][1] == 2) and (run_data[i + 1][1] == 2)):
                if (run_data[i][4] != run_data[i + 1][4]):
                    break
                basf2.B2INFO('Merging run %d (not enough data) into '
                             'run %d (not enough data).' %
                             (run_data[i + 1][0], run_data[i][0]))
                run_data[i][2].extend(run_data[i + 1][2])
                del run_data[i + 1]
                self.execute_over_run_list(run_data[i][2], iteration, False)
                run_data[i][1] = self.machine.result.result
                run_data[i][3] = KLMChannelStatusAlgorithm.Results(
                    self.machine.algorithm.algorithm.getResults())
                run_data[i][5] = \
                    self.machine.algorithm.algorithm.getPayloadValues()
                result_str = calibration_result_string(run_data[i][1])
                basf2.B2INFO('Run %d: %s.' % (run_data[i][0], result_str))
                if (i >= len(run_data) - 1):
                    break
            i += 1

        # Merge runs that do not have enough data into normal runs.
        # Currently merging the data (TODO: consider result comparison).
        def merge_runs(run_data, run_not_enough_data, run_normal, forced):
            basf2.B2INFO('Merging run %d (not enough data) into '
                         'run %d (normal).' %
                         (run_data[run_not_enough_data][0],
                          run_data[run_normal][0]))
            run_data[run_normal][2].extend(run_data[run_not_enough_data][2])
            self.execute_over_run_list(run_data[run_normal][2], iteration,
                                       forced)
            run_data[run_normal][1] = self.machine.result.result
            run_data[run_normal][3] = KLMChannelStatusAlgorithm.Results(
                self.machine.algorithm.algorithm.getResults())
            run_data[run_normal][5] = self.machine.algorithm.algorithm.getPayloadValues()
            result_str = calibration_result_string(run_data[run_normal][1])
            basf2.B2INFO('Run %d: %s.' % (run_data[run_normal][0], result_str))
            if (run_data[run_normal][1] != 0):
                basf2.B2FATAL('Merging run %d into run %d failed.' %
                              (run_data[run_not_enough_data][0],
                               run_data[run_normal][0]))
            del run_data[run_not_enough_data]

        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == 2):
                if (run_data[i][4] == 'next'):
                    merge_runs(run_data, i, i + 1, False)
                elif (run_data[i][4] == 'previous'):
                    merge_runs(run_data, i, i - 1, False)
                else:
                    i += 1
            else:
                i += 1
        i = 0
        while (i < len(run_data)):
            if (run_data[i][1] == 2 and run_data[i][4] == 'none'):
                new_modules_previous = -1
                new_modules_next = -1
                if (i < len(run_data) - 1):
                    new_modules_next = run_data[i][3].getModuleStatus(). \
                        newNormalChannels(run_data[i + 1][3].getModuleStatus())
                    basf2.B2INFO('There are %d new active modules in run %d '
                                 'relatively to run %d.' %
                                 (new_modules_next, run_data[i][0],
                                  run_data[i + 1][0]))
                if (i > 0):
                    new_modules_previous = run_data[i][3].getModuleStatus(). \
                        newNormalChannels(run_data[i - 1][3].getModuleStatus())
                    basf2.B2INFO('There are %d new active modules in run %d '
                                 'relatively to run %d.' %
                                 (new_modules_previous, run_data[i][0],
                                  run_data[i - 1][0]))
                run_for_merging = -1
                # If a forced merge of the normal run with another run from
                # a different range of runs with not enough data has already
                # been performed, then the list of active modules may change
                # and there would be 0 new modules. Consequently, the number
                # of modules is checked to be greater or equal than 0. However,
                # there is no guarantee that the same added module would be
                # calibrated normally. Thus, a forced merge is performed anyway.
                if (new_modules_previous >= 0 and new_modules_next < 0):
                    run_for_merging = i - 1
                elif (new_modules_previous < 0 and new_modules_next >= 0):
                    run_for_merging = i + 1
                elif (new_modules_previous >= 0 and new_modules_next >= 0):
                    if (new_modules_previous < new_modules_next):
                        run_for_merging = i - 1
                    else:
                        run_for_merging = i + 1
                else:
                    basf2.B2INFO('Cannot determine run for merging for run %d, '
                                 'performing its forced calibration.' %
                                 (run_data[i][0]))
                    self.execute_over_run_list(run_data[i][2], iteration, True)
                    run_data[i][1] = self.machine.result.result
                    run_data[i][3] = KLMChannelStatusAlgorithm.Results(
                        self.machine.algorithm.algorithm.getResults())
                    run_data[i][5] = self.machine.algorithm.algorithm.getPayloadValues()
                    result_str = calibration_result_string(run_data[i][1])
                    basf2.B2INFO('Run %d: %s.' % (run_data[i][0], result_str))
                    if (run_data[i][1] != 0):
                        basf2.B2FATAL('Forced calibration of run %d failed.' %
                                      (run_data[i][0]))
                if (run_for_merging >= 0):
                    merge_runs(run_data, i, run_for_merging, True)
            else:
                i += 1

        # Write the results.
        def commit_payload(run_data):
            if (run_data[1] == 2):
                basf2.B2INFO('Run %d has no calibration result, skipped.' %
                             (run_data[0]))
                return
            basf2.B2INFO('Writing run %d.' % (run_data[0]))
            self.machine.algorithm.algorithm.commit(run_data[5])

        def write_result(run_data, run):
            iov = run_data[run][5].front().iov
            run_low = iov.getRunLow()
            run_high = iov.getRunHigh()
            j = 0
            runs = []
            while (j < len(run_data_klm_excluded)):
                if (run_low < run_data_klm_excluded[j][0] and
                        ((run_data_klm_excluded[j][0] < run_high) or
                         (run_high == -1))):
                    runs.append([run_data_klm_excluded[j][0], 'klm_excluded'])
                j += 1
            if (len(runs) == 0):
                commit_payload(run_data[run])
                return
            for r in run_data[run][2]:
                runs.append([r.run, 'klm_included'])
            runs.sort(key=lambda x: x[0])
            run_first = 0
            run_last = 0
            while (run_last < len(runs)):
                run_last = run_first
                while (runs[run_last][1] == runs[run_first][1]):
                    run_last += 1
                    if (run_last >= len(runs)):
                        break
                if (run_first == 0):
                    run1 = run_low
                else:
                    run1 = runs[run_first][0]
                if (run_last < len(runs)):
                    run2 = runs[run_last][0] - 1
                else:
                    run2 = run_high
                iov = Belle2.IntervalOfValidity(experiment, run1,
                                                experiment, run2)
                if (runs[run_first][1] == 'klm_included'):
                    run_data[run][5].front().iov = iov
                    commit_payload(run_data[run])
                else:
                    run_data_klm_excluded[0][5].front().iov = iov
                    commit_payload(run_data_klm_excluded[0])
                run_first = run_last

        first_run = 0
        for i in range(0, len(run_data)):
            # Get first run again due to possible mergings.
            run_data[i][2].sort(key=lambda x: x.run)
            first_run = run_data[i][2][0].run
            # Set IOV for the current run.
            # The last run will be overwritten when writing the result.
            run_data[i][5].front().iov = \
                Belle2.IntervalOfValidity(experiment, first_run, experiment, -1)
            # Compare with the previous run.
            write_previous_run = True
            if (i > 0):
                if (run_data[i][1] == 0 and run_data[i - 1][1] == 0):
                    if (run_data[i][3].getChannelStatus() ==
                            run_data[i - 1][3].getChannelStatus()):
                        basf2.B2INFO('Run %d: result is the same as '
                                     'for the previous run %d.' %
                                     (run_data[i][0], run_data[i - 1][0]))
                        if (previous_run >= 0):
                            iov = run_data[previous_run][5].front().iov
                            run_data[previous_run][5].front().iov = \
                                Belle2.IntervalOfValidity(
                                    experiment, iov.getRunLow(),
                                    experiment, first_run - 1)
                            write_previous_run = False
            # Set IOV for the current run.
            # The last run will be overwritten when writing the result.
            run_data[i][5].front().iov = Belle2.IntervalOfValidity(experiment, first_run, experiment, -1)
            # If the calibration result is different, write the previous run.
            if (write_previous_run and (i > 0)):
                iov = run_data[previous_run][5].front().iov
                if (previous_run == 0):
                    run_data[previous_run][5].front().iov = Belle2.IntervalOfValidity(
                        lowest_exprun.exp, lowest_exprun.run,
                        experiment, first_run - 1)
                else:
                    run_data[previous_run][5].front().iov = Belle2.IntervalOfValidity(experiment, iov.getRunLow(),
                                                                                      experiment, first_run - 1)
                write_result(run_data, previous_run)
                previous_run = i
            if (i == 0):
                previous_run = 0
            # Write the current run if it is the last run.
            if (i == len(run_data) - 1):
                iov = run_data[i][5].front().iov
                run_data[i][5].front().iov = Belle2.IntervalOfValidity(
                    experiment, iov.getRunLow(),
                    highest_exprun.exp, highest_exprun.run)
                write_result(run_data, i)
