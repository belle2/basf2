##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""CDC fudge factor calibration."""
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from prompt.calibrations.caf_cdc import settings as cdc_tracking_calibration
from prompt.calibrations.caf_vxdcdc_alignment import settings as full_alignment
from prompt.utils import ExpRun
import basf2
from ROOT import Belle2
from caf.framework import Calibration
from caf import strategies
from modularAnalysis import fillParticleList, cutAndCopyList, reconstructDecay, applyCuts
from vertex import treeFit

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC Sigma fudge factor",
                               expert_username="dongthanh.ht",
                               subsystem="cdc",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["mumu_tight_or_highm_calib"],
                               input_data_filters={"mumu_tight_or_highm_calib":
                                                   [INPUT_DATA_FILTERS["Data Tag"]["mumu_tight_or_highm_calib"],
                                                    INPUT_DATA_FILTERS["Data Quality Tag"]["Good"],
                                                    INPUT_DATA_FILTERS["Magnet"]["On"]]},
                               depends_on=[cdc_tracking_calibration, full_alignment],
                               expert_config={
                                   "fileFormat": "RAW",
                                   "min_events_per_file": 500,
                                   "max_events_per_file": 30000,
                                   "components": ["CDC", "ECL", "KLM"],
                                   "vertex_fit": 0,
                                    "payload_boundaries": [],
                                   "backend_args": {"request_memory": "4 GB"}
                               },
                               produced_payloads=["CDCFudgeFactorsForSigma"])


################################################
# Required function called by b2caf-prompt-run #
################################################

def get_calibrations(input_data, **kwargs):
    # Set up config options
    # read expert_config values
    expert_config = kwargs.get("expert_config")
    min_events_per_file = expert_config["min_events_per_file"]
    max_events_per_file = expert_config["max_events_per_file"]
#    file_format = expert_config["file_format"]
    components = expert_config["components"]
    vertex_fit = expert_config["vertex_fit"]
    fileFormat = expert_config["fileFormat"]
    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_mumu = input_data["mumu_tight_or_highm_calib"]

    from prompt.utils import filter_by_max_files_per_run
    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, 100, min_events_per_file)
    input_files_mumu = list(reduced_file_to_iov_mumu.keys())
    basf2.B2INFO("Complete input data selection.")
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_mumu)}")

    payload_boundaries = []
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Payload boundaries from expert_config: {payload_boundaries}")

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # for SingleIOV stratrgy, it's better to set the granularity to 'all' so that the collector jobs will run faster
    collector_granularity = 'all'
    if payload_boundaries:
        basf2.B2INFO('Found payload_boundaries: set collector granularity to run')
        collector_granularity = 'run'

    ###################################################
    # call collector module
    col = basf2.register_module("CDCFudgeFactorCalibrationCollector",
                                granularity=collector_granularity)
    # call algorighm
    algo = Belle2.CDC.FudgeFactorCalibrationAlgorithm()
    algo.setHistFileName("histo_fudge_factor.root")
    ###################################################
    # Calibration setup
    fudge_calib = Calibration("CDC_FudgeFactor",
                              collector=col,
                              algorithms=algo,
                              input_files=input_files_mumu,
                              pre_collector_path=pre_collector(max_events_per_file,
                                                               components=components,
                                                               fileFormat=fileFormat,
                                                               vertex_fit=vertex_fit))
#                              backend_args=expert_config["backend_args"])
    #                           collector_granularity=collector_granularity)
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    if payload_boundaries:
        basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        fudge_calib.strategies = strategies.SequentialBoundaries
        for alg in fudge_calib.algorithms:
            alg.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        for alg in fudge_calib.algorithms:
            alg.params = {"apply_iov": output_iov}

    return [fudge_calib]


#################################################
def pre_collector(max_events=None, components=["CDC", "ECL", "KLM"], fileFormat="RAW", vertex_fit=0):
    """
    Define pre collection (reconstruction in our purpose).
    Probably, we need only CDC and ECL data.
    Parameters:
        max_events [int] : number of events to be processed.
                           All events by Default.
    Returns:
        path : path for pre collection
    """
    from basf2 import create_path, register_module
    from softwaretrigger.constants import HLT_INPUT_OBJECTS
    reco_path = create_path()
    if fileFormat == "RAW":
        if max_events is None:
            root_input = register_module('RootInput', branchNames=HLT_INPUT_OBJECTS)
        else:
            root_input = register_module('RootInput', branchNames=HLT_INPUT_OBJECTS,
                                         entrySequences='0:{}'.format(max_events))
        reco_path.add_module(root_input)
        # unpack
        from rawdata import add_unpackers
        from reconstruction import add_reconstruction
        add_unpackers(reco_path, components=components)
        # reconstruction
        add_reconstruction(reco_path,
                           components=components,
                           append_full_grid_cdc_eventt0=True,
                           skip_full_grid_cdc_eventt0_if_svd_time_present=False)
    if fileFormat == "mdst":
        from modularAnalysis import inputMdst
        inputMdst(filename="", path=reco_path, environmentType='default', skipNEvents=0, entrySequence=['0:{}'.format(max_events)])

    reco_path.add_module('Progress')
    fillParticleList('gamma:HLT', 'E>0.1', path=reco_path)
    goodTrack = 'abs(d0) < 2.0 and abs(z0) < 4.0 and pt > 2.0 and useCMSFrame(p) > 0.5'
    fillParticleList('mu+:HLT', goodTrack, path=reco_path)
    cutAndCopyList('mu+:sel', 'mu+:HLT', goodTrack, path=reco_path)
    reconstructDecay('vpho:mumu -> mu+:sel mu-:sel', '', path=reco_path)  # apply event cuts
    applyCuts('vpho:mumu', '[nCleanedTracks('+goodTrack+') == 2]', path=reco_path)
    if vertex_fit == 1:
        treeFit('vpho:mumu', ipConstraint=False, updateAllDaughters=False, path=reco_path)

    return reco_path
