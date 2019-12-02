# -*- coding: utf-8 -*-

"""Implements some extra utilities for doing KLM calibration with the CAF."""

import basf2

from rawdata import add_unpackers
from reconstruction import add_cosmics_reconstruction, add_reconstruction
import modularAnalysis as ma

from caf.strategies import AlgorithmStrategy
from caf.utils import IoV, AlgResult
from caf.utils import runs_overlapping_iov, runs_from_vector


class KLMStripEfficiency(AlgorithmStrategy):
    """Custom strategy for executing the KLM strip efficiency. Requires complex run merging rules.

    This uses a `caf.state_machines.AlgorithmMachine` to actually execute the various steps rather than operating on
    a CalibrationAlgorithm C++ class directly."""

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

        B2INFO("Setting up {} strategy for {}".format(self.__class__.__name__, self.algorithm.name))
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
        B2INFO("Starting AlgorithmMachine of {}".format(self.algorithm.name))
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        B2INFO("Beginning execution of {} using strategy {}".format(self.algorithm.name, self.__class__.__name__))

        all_runs_collected = set(runs_from_vector(self.algorithm.algorithm.getRunListFromAllData()))
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            B2INFO("Removing the ignored_runs from the runs to execute for {}".format(self.algorithm.name))
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)
        apply_iov = None
        if "apply_iov" in self.algorithm.params:
            apply_iov = self.algorithm.params["apply_iov"]
        self.machine.execute_runs(runs=runs_to_execute, iteration=iteration, apply_iov=apply_iov)
        B2INFO("Finished execution with result code {}".format(self.machine.result.result))

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


def get_cosmic_pre_collector_path(entry_sequence=""):
    """
    Parameters:
        entry_sequence  (str): A single entry sequence e.g. '0:100' to help limit processed events.

    Returns:
        basf2.Path:  A reconstruction path to run before the collector. Used for raw cosmic input files.
    """
    main = basf2.create_path()
    if entry_sequence:
        root_input = basf2.register_module("RootInput", entrySequences=[entry_sequence])
        main.add_module(root_input)

    main.add_module('Gearbox')
    main.add_module('Geometry')

    # Unpackers and reconstruction.
    add_unpackers(main)
    add_cosmics_reconstruction(main, merge_tracks=False)

    # Fill muon particle list
    ma.fillParticleList('mu+:all', '1 < p and p < 11', path=main)

    return main


def get_physics_pre_collector_path(entry_sequence=""):
    """
    Parameters:
        entry_sequence  (str): A single entry sequence e.g. '0:100' to help limit processed events.

    Returns:
        basf2.Path:  A reconstruction path to run before the collector. Used for raw physics input files.
    """
    main = basf2.create_path()
    if entry_sequence:
        root_input = basf2.register_module("RootInput", entrySequences=[entry_sequence])
        main.add_module(root_input)
    main.add_module('Gearbox')
    main.add_module('Geometry')

    # HLT selection.
    empty_path = basf2.create_path()
    trigger_skim = basf2.register_module('TriggerSkim')
    trigger_skim.param('triggerLines', 'software_trigger_cut&skim&accept_mumu_2trk')
    trigger_skim.if_false(empty_path, basf2.AfterConditionPath.END)
    main.add_module(trigger_skim)

    # Unpackers and reconstruction.
    add_unpackers(main)
    add_reconstruction(main)
    ma.fillParticleList('mu+:all', '1 < p and p < 11 and d0 < 2 and abs(z0) < 5',
                        path=main)

    return main


def get_cdst_pre_collector_path(entry_sequence=""):
    """
    Parameters:
        entry_sequence  (str): A single entry sequence e.g. '0:100' to help limit processed events.

    Returns:
        basf2.Path:  A reconstruction path to run before the collector. Used for cDST input files.
    """
    main = basf2.create_path()
    if entry_sequence:
        root_input = basf2.register_module("RootInput", entrySequences=[entry_sequence])
        main.add_module(root_input)
    main.add_module('Gearbox')
    main.add_module('Geometry')

    # Fill muon particle list
    ma.fillParticleList('mu+:all', '1 < p and p < 11', path=main)

    return main
