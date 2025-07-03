##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""CDC badwire calibration."""
import basf2
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from prompt.calibrations.caf_cdc import settings as cdc_tracking_calibration
from ROOT import Belle2

settings = CalibrationSettings(name="CDC badwire",
                               expert_username="manhtt",
                               subsystem_responsible_gitlab_username="dongthanh.ht",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["mumu_tight_or_highm_calib"],
                               input_data_filters={"mumu_tight_or_highm_calib":
                                                   [INPUT_DATA_FILTERS["Data Tag"]["mumu_tight_or_highm_calib"],
                                                    INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                                    INPUT_DATA_FILTERS["Magnet"]["On"]]},
                               depends_on=[cdc_tracking_calibration],
                               expert_config={
                                   "min_events_per_file": 500,
                                   "max_events_per_file": 10000,
                                   "components": ["CDC", "ECL", "KLM"],
                                    "payload_boundaries": [],
                                   "backend_args": {"request_memory": "4 GB"}
                               },
                               produced_payloads=["CDCBadWires"])


# Main function to get calibrations
def get_calibrations(input_data, **kwargs):
    expert_config = kwargs.get("expert_config")
    min_events_per_file = expert_config["min_events_per_file"]
    max_events_per_file = expert_config["max_events_per_file"]
    components = expert_config["components"]

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_mumu = input_data["mumu_tight_or_highm_calib"]
    from prompt.utils import filter_by_max_files_per_run
    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, 100, min_events_per_file)
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
    col = basf2.register_module("CDCBadWireCollector")

    # call algorighm
    algo = Belle2.CDC.WireEfficiencyAlgorithm()
    algo.setInputFileNames("histo_badwire.root")
    # Calibration setup
    from caf.framework import Calibration
    badwire_calib = Calibration("CDC_Badwire",
                                collector=col,
                                algorithms=algo,
                                input_files=input_files_mumu,
                                pre_collector_path=pre_collector(max_events_per_file,
                                                                 components=components))
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    if payload_boundaries:
        basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        badwire_calib.strategies = strategies.SequentialBoundaries
        for alg in badwire_calib.algorithms:
            alg.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        for alg in badwire_calib.algorithms:
            alg.params = {"apply_iov": output_iov}

    return [badwire_calib]


def pre_collector(max_events=None, components=["CDC", "ECL", "KLM"]):
    from rawdata import add_unpackers
    # Create an execution path
    path = basf2.create_path()
    branches = ['EventMetaData', 'RawCDCs', 'RawFTSWs']
    unpackers = ['CDC']
    path.add_module("RootInput", branchNames=branches)
    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)
    path.add_module("SetupGenfitExtrapolation",
                    energyLossBrems=False, noiseBrems=False)
    add_unpackers(path, components=unpackers)

    # Print some progress messages
    path.add_module("Progress")

    from reconstruction import default_event_abort, add_prefilter_pretracking_reconstruction
    from tracking import add_prefilter_tracking_reconstruction

    # Do not even attempt at reconstructing events w/ abnormally large occupancy.
    doom = path.add_module("EventsOfDoomBuster")
    default_event_abort(doom, ">=1", Belle2.EventMetaData.c_ReconstructionAbort)
    path.add_module('StatisticsSummary').set_name('Sum_EventsofDoomBuster')

    Components = ["CDC"]
    # Add modules that have to be run BEFORE track reconstruction
    add_prefilter_pretracking_reconstruction(path, components=Components)

    # Add tracking reconstruction modules
    add_prefilter_tracking_reconstruction(path=path,
                                          components=Components,
                                          trackFitHypotheses=[211],
                                          prune_temporary_tracks=False,
                                          fit_tracks=True,
                                          append_full_grid_cdc_eventt0=True,
                                          skip_full_grid_cdc_eventt0_if_svd_time_present=False)
    path.add_module('StatisticsSummary').set_name('Sum_Tracking')

    # Making sure CDC Raw Hits are stored
    for module in path.modules():
        if module.name() == 'CDCUnpacker':
            print('Enabling Raw CDC hits')
            module.param({'enableStoreCDCRawHit': True})
        if module.name() == 'TFCDC_WireHitPreparer':
            print('Enabling bad wires')
            module.param({'useBadWires': True})

    # Print a textual representation of what was just put together
    basf2.print_path(path)
    return path
