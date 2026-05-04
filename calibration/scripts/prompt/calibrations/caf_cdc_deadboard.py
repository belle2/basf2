##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""CDC deadboard calibration."""
import basf2
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from ROOT import Belle2

settings = CalibrationSettings(name="CDC deadboard",
                               expert_username="liumh",
                               subsystem="cdc",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["mumu_tight_or_highm_calib"],
                               input_data_filters={"mumu_tight_or_highm_calib":
                                                   [INPUT_DATA_FILTERS["Data Tag"]["mumu_tight_or_highm_calib"],
                                                    INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                                    INPUT_DATA_FILTERS["Magnet"]["On"]]},
                               depends_on=[],
                               expert_config={
                                   "min_events_per_file": 100,
                                   "components": ["CDC"],
                                   "payload_boundaries": [],
                                   "backend_args": {"request_memory": "2 GB"},
                                   "dead_board_threshold": 0.0
                               },
                               produced_payloads=["CDCBadBoards"])


# Main function to get calibrations
def get_calibrations(input_data, **kwargs):
    expert_config = kwargs.get("expert_config")
    min_events_per_file = expert_config["min_events_per_file"]
    components = expert_config["components"]
    dead_board_threshold = expert_config["dead_board_threshold"]

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_mumu = input_data["mumu_tight_or_highm_calib"]
    from prompt.utils import filter_by_max_files_per_run
    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, 1, min_events_per_file)
    input_files_mumu = list(reduced_file_to_iov_mumu.keys())
    basf2.B2INFO("Complete input data selection.")
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_mumu)}")

    from prompt.utils import ExpRun
    from caf.utils import IoV
    from caf import strategies
    payload_boundaries = []
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Payload boundaries from expert_config: {payload_boundaries}")

    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # for SingleIOV stratrgy, it's better to set the granularity to 'all' so that the collector jobs will run faster
    if payload_boundaries:
        basf2.B2INFO('Found payload_boundaries: set collector granularity to run')
    # call collector module
    col = basf2.register_module("CDCDeadBoardDetector")

    # call algorighm
    algo = Belle2.CDC.DeadBoardAlgorithm()
    algo.setHistName("CDCboardIDs")
    algo.setThreshold(dead_board_threshold)
    # Calibration setup
    from caf.framework import Calibration
    deadboard_calib = Calibration("CDC_DeadBoard",
                                  collector=col,
                                  algorithms=algo,
                                  input_files=input_files_mumu,
                                  pre_collector_path=pre_collector(components=components))
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    if payload_boundaries:
        basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        deadboard_calib.strategies = strategies.SequentialBoundaries
        for alg in deadboard_calib.algorithms:
            alg.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        deadboard_calib.strategies = strategies.SequentialRunByRun
        for alg in deadboard_calib.algorithms:
            alg.params = {"iov_coverage": output_iov}

    return [deadboard_calib]


def pre_collector(components=["CDC"]):
    # Create an execution path
    path = basf2.create_path()
    branches = ['EventMetaData', 'RawCDCs', 'RawFTSWs']

    # Only rootInput is required with RawCDCs included:
    path.add_module("RootInput", branchNames=branches)
    # Print some progress messages
    path.add_module("Progress")

    basf2.print_path(path)
    return path
