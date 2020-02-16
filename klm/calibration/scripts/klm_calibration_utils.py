# -*- coding: utf-8 -*-

"""Implements some extra utilities for doing KLM calibration with the CAF."""

import basf2

from rawdata import add_unpackers
from reconstruction import add_cosmics_reconstruction, add_reconstruction
import modularAnalysis as ma

from caf.utils import IoV, AlgResult
from caf.utils import runs_overlapping_iov, runs_from_vector
from caf.strategies import AlgorithmStrategy
from caf.state_machines import AlgorithmMachine
from ROOT.Belle2 import KLMStripEfficiencyAlgorithm


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
        #: The multiprocessing queue we use to pass back results one at a time
        self.queue = queue

        basf2.B2INFO(f"Setting up {self.__class__.__name__} strategy for {self.algorithm.name}")
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
        basf2.B2INFO(f"Starting AlgorithmMachine of {self.algorithm.name}")
        self.algorithm.algorithm.setCalibrationStage(
            KLMStripEfficiencyAlgorithm.c_EfficiencyMeasurement)
        # This sets up the logging and database chain, plus it assigns all input files from collector jobs.
        # Note that we simply assign all input files, and use the execute(runs) to define which data to use.
        self.machine.setup_algorithm(iteration=iteration)
        # After this point, the logging is in the stdout of the algorithm
        basf2.B2INFO(f"Beginning execution of {self.algorithm.name} using strategy {self.__class__.__name__}")

        all_runs_collected = set(runs_from_vector(self.algorithm.algorithm.getRunListFromAllData()))
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        if iov:
            runs_to_execute = runs_overlapping_iov(iov, all_runs_collected)
        else:
            runs_to_execute = all_runs_collected

        # Remove the ignored runs from our run list to execute
        if self.ignored_runs:
            basf2.B2INFO(f"Removing the ignored_runs from the runs to execute for {self.algorithm.name}")
            runs_to_execute.difference_update(set(self.ignored_runs))
        # Sets aren't ordered so lets go back to lists and sort
        runs_to_execute = sorted(runs_to_execute)

        apply_iov = None
        if "apply_iov" in self.algorithm.params:
            apply_iov = self.algorithm.params["apply_iov"]

        # apply_iov forces the execute_runs() function to use this IoV as the value for saveCalibration()
        self.machine.execute_runs(runs=runs_to_execute, iteration=iteration, apply_iov=apply_iov)
        basf2.B2INFO(f"Finished execution with result code {self.machine.result.result}")

        # At this point you can test the result code and decide if you want to actually commit payloads using
        #
        # self.machine.result
        #
        # If you want to save the payloads for later and commit them use:
        #
        # payloads = self.machine.algorithm.algorithm.getPayloadValues()
        #
        # and then to commit them to localdb later:
        #
        # self.machine.algorithm.algorithm.commit(payloads)
        #
        # To execute again, just complete or fail: and do setup again. This will NOT re-run the loggin setup etc.
        #
        # self.machine.complete()
        # self.machine.setup_algorithm()
        # self.machine.execute_runs(runs=runs_to_execute, iteration=iteration, apply_iov=apply_iov)

        # Whenever you have a final result that can't be changed e.g. a success you committed, or a failure you can't
        # force past or merge runs to avoid. You should send that result out.
        self.send_result(self.machine.result)

        # Check the result and commit
        if (self.machine.result.result == AlgResult.ok.value) or (self.machine.result.result == AlgResult.iterate.value):
            # Valid exit codes mean we can complete properly
            self.machine.complete()
            # Commit all the current payloads (didn't bother saving them for later)
            self.machine.algorithm.algorithm.commit()
            # Tell the controlling process that we are done and whether the overall algorithm process managed to succeed
            self.send_final_state(self.COMPLETED)
        else:
            # Either there wasn't enough data or the algorithm failed
            self.machine.fail()
            # Tell the controlling process that we are done and whether the overall algorithm process managed to succeed
            self.send_final_state(self.FAILED)


def get_alignment_pre_collector_path_cosmic(entry_sequence=""):
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
    add_cosmics_reconstruction(main, pruneTracks=False, add_muid_hits=True,
                               merge_tracks=False)
    # Disable the time window in muid module by setting it to 1 second.
    # This is necessary because the  alignment needs to be performed before
    # the time calibration; if the time window is not disabled, then all
    # scintillator hits are rejected.
    basf2.set_module_parameters(main, 'Muid', MaxDt=1e9)

    main.add_module("MergerCosmicTracks",
                    recoTracksStoreArrayName="RecoTracks",
                    mergedRecoTracksStoreArrayName="CosmicRecoTracks",
                    usingMagneticField=True)

    main.add_module('DAFRecoFitter',
                    recoTracksStoreArrayName='CosmicRecoTracks',
                    pdgCodesToUseForFitting=[13],
                    resortHits=True)

    main.add_module("TrackCreator",
                    recoTrackColName="CosmicRecoTracks",
                    trackColName="CosmicTracks",
                    trackFitResultColName="CosmicTrackFitResults",
                    useClosestHitToIP=True)

    main.add_module('SetupGenfitExtrapolation',
                    noiseBetheBloch=False,
                    noiseCoulomb=False,
                    noiseBrems=False)

    return main


def get_alignment_pre_collector_path_physics(entry_sequence=""):
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

    # Unpackers and reconstruction.
    add_unpackers(main)
    add_reconstruction(main, pruneTracks=False, add_muid_hits=True)
    # Disable the time window in muid module by setting it to 1 second.
    # This is necessary because the  alignment needs to be performed before
    # the time calibration; if the time window is not disabled, then all
    # scintillator hits are rejected.
    basf2.set_module_parameters(main, 'Muid', MaxDt=1e9)

    main.add_module('DAFRecoFitter', resortHits=True)

    main.add_module('SetupGenfitExtrapolation',
                    noiseBetheBloch=False,
                    noiseCoulomb=False,
                    noiseBrems=False)

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
    ma.fillParticleList('mu+:all',
                        '1 < p and p < 11 and abs(d0) < 2 and abs(z0) < 5',
                        path=main)

    return main
