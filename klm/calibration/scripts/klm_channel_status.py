# -*- coding: utf-8 -*-

"""Custom calibration strategy for KLM channel status."""

import collections
import numpy
import os

import basf2
import ROOT

from caf.utils import ExpRun, IoV, AlgResult
from caf.utils import runs_overlapping_iov, runs_from_vector
from caf.utils import split_runs_by_exp
from caf.strategies import AlgorithmStrategy
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
            if (algorithm_results.getTotalHitNumber() > 0):
                run_data.append([exp_run.run, result, [exp_run],
                                 algorithm_results, '', None])
            else:
                run_data_klm_excluded.append([exp_run.run, result, [exp_run],
                                              algorithm_results, '', None])
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
        hit_map_module.Branch('active_channels', active_channels, 'active_channels/I')
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
