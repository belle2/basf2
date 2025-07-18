##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


"""
Airflow script for automatic CDC dEdx calibration. It is the hadron based
calibration, where Hadron saturation and beta gamma curve and resolution calibration implemented.
"""

from prompt import CalibrationSettings, INPUT_DATA_FILTERS
from caf.framework import Calibration, Collection
from caf.strategies import SequentialBoundaries
from ROOT.Belle2 import CDCDedxHadSatAlgorithm, CDCDedxHadBGAlgorithm
from basf2 import create_path, register_module
from cdcdedx_calibration_utils import make_pion_kaon_collection, make_proton_collection
from cdcdedx_calibration_utils import make_muon_collection, make_electron_collection

from prompt.calibrations.caf_cdcdedx_electron import settings as cdc_dedx_electron
from prompt.calibrations.caf_top import settings as top


quality_flags = [INPUT_DATA_FILTERS["Run Type"]["physics"],
                 INPUT_DATA_FILTERS["Data Quality Tag"]["Good Or Recoverable"],
                 INPUT_DATA_FILTERS["Magnet"]["On"],
                 INPUT_DATA_FILTERS['Beam Energy']['4S'],
                 INPUT_DATA_FILTERS['Beam Energy']['Continuum'],
                 INPUT_DATA_FILTERS['Beam Energy']['Scan']]

collection = ["pion_kaon", "electron", "muon", "proton"]

#: Tells the automated system some details of this script
settings = CalibrationSettings(
    name="CDC dedx Hadron Calibrations",
    expert_username="renu92garg",
    subsystem="cdc",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["hadron_calib", "bhabha_all_calib", "radmumu_calib"],
    input_data_filters={
      "hadron_calib": [INPUT_DATA_FILTERS["Data Tag"]["hadron_calib"]] + quality_flags,
      "bhabha_all_calib": [INPUT_DATA_FILTERS['Data Tag']['bhabha_all_calib']] + quality_flags,
      "radmumu_calib": [INPUT_DATA_FILTERS['Data Tag']['radmumu_calib']] + quality_flags
      },
    expert_config={"payload_boundaries": [],
                   "proton_minCut": 0.5,
                   "proton_maxCut": 1.2,
                   "maxevt": 2e6,
                   "sat_bgpar": {"muon": [8, 2.83, 28.83], "proton": [6, 0.44, 0.85]},
                   "bgpar": {"muon": [12, 2.85, 28.85], "proton": [20, 0.33, 0.85]},
                   "cosbins": {"muon": 24, "proton": 20},
                   },
    depends_on=[cdc_dedx_electron, top],
    produced_payloads=["CDCDedxHadronCor", "CDCDedxMeanPars", "CDCDedxSigmaPars"])

##############################


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["output_iov"] is used. This is the output IoV range that your payloads should
        correspond to. Generally your highest ExpRun payload should be open ended e.g. IoV(3,4,-1,-1)

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    import basf2

    # Set up config options
    # In this script we want to use one sources of input data.
    # Get the input files from the input_data variable
    file_to_iov_muon = input_data["radmumu_calib"]
    file_to_iov_electron = input_data["bhabha_all_calib"]
    file_to_iov_hadron = input_data["hadron_calib"]

    expert_config = kwargs.get("expert_config")
    maxevt = expert_config["maxevt"]

    from prompt.utils import filter_by_select_max_events_from_files

    input_files_muon = list(file_to_iov_muon.keys())

    input_files_electron = filter_by_select_max_events_from_files(list(file_to_iov_electron.keys()), maxevt)
    if not input_files_electron:
        raise ValueError(f"Electron: all requested ({maxevt}) events not found")

    input_files_hadron = list(file_to_iov_hadron.keys())

    basf2.B2INFO(f"Total number of files actually used as input for muon = {len(input_files_muon)}")
    basf2.B2INFO(f"Total number of files actually used as input for electron = {len(input_files_electron)}")
    basf2.B2INFO(f"Total number of files actually used as input for hadron = {len(input_files_hadron)}")

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import ExpRun, IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    payload_boundaries = [ExpRun(output_iov.exp_low, output_iov.run_low)]
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    basf2.B2INFO(f"Expert set payload boundaries are: {expert_config['payload_boundaries']}")

    # extracting parameters
    proton_minCut = expert_config["proton_minCut"]
    proton_maxCut = expert_config["proton_maxCut"]
    bgpar = expert_config["bgpar"]
    sat_bgpar = expert_config["sat_bgpar"]
    cosbins = expert_config["cosbins"]

    ###################################################
    # Calibration setup

    root_input = register_module("RootInput")

    #
    # pion kaon collection
    #
    rec_path_pion_kaon = create_path()
    rec_path_pion_kaon.add_module(root_input)

    pion_kaon_list = []
    pion_kaon_list = make_pion_kaon_collection(rec_path_pion_kaon)

    collector_pion_kaon = register_module('CDCDedxHadronCollector')
    collector_pion_kaon.param("particleLists", pion_kaon_list)

    collection_pion_kaon = Collection(collector=collector_pion_kaon,
                                      input_files=input_files_hadron,
                                      pre_collector_path=rec_path_pion_kaon, max_collector_jobs=400)

    #
    # proton collection
    #
    rec_path_proton = create_path()
    rec_path_proton.add_module(root_input)
    proton_list = make_proton_collection(rec_path_proton)

    collector_proton = register_module('CDCDedxHadronCollector', particleLists=proton_list)
    collector_proton.param("maxcut", proton_maxCut)
    collector_proton.param("mincut", proton_minCut)

    collection_proton = Collection(collector=collector_proton,
                                   input_files=input_files_hadron,
                                   pre_collector_path=rec_path_proton,
                                   max_collector_jobs=400
                                   )

    #
    # muon collection
    #
    rec_path_muon = create_path()
    rec_path_muon.add_module(root_input)
    muon_list = make_muon_collection(rec_path_muon)

    collector_muon = register_module('CDCDedxHadronCollector', particleLists=muon_list)

    collection_muon = Collection(collector=collector_muon,
                                 input_files=input_files_muon,
                                 pre_collector_path=rec_path_muon
                                 )

    #
    # electron collection
    #
    rec_path_electron = create_path()
    rec_path_electron.add_module(root_input)
    electron_list = make_electron_collection(rec_path_electron)
    collector_electron = register_module('CDCDedxHadronCollector', particleLists=electron_list)

    collection_electron = Collection(collector=collector_electron,
                                     input_files=input_files_electron,
                                     pre_collector_path=rec_path_electron
                                     )

    # set algorithim
    algorithm_sat = CDCDedxHadSatAlgorithm()
    algorithm_sat.setProtonCut(proton_minCut)
    for key, value in sat_bgpar.items():
        algorithm_sat.setBGPars(key, value[0], value[1], value[2])

    algorithm_bg = CDCDedxHadBGAlgorithm()
    algorithm_bg.setProtonCut(proton_minCut)

    for key, value in bgpar.items():
        algorithm_bg.setBGPars(key, value[0], value[1], value[2])

    for key, value in cosbins.items():
        algorithm_bg.setCosBin(key, value)

    # set calibration
    cal = Calibration("Hadron_saturation")
    cal.add_collection(name="pion_kaon", collection=collection_pion_kaon)
    cal.add_collection(name="proton", collection=collection_proton)
    cal.add_collection(name="muon", collection=collection_muon)
    cal.add_collection(name="electron", collection=collection_electron)

    cal.algorithms = [algorithm_sat, algorithm_bg]

    if payload_boundaries:
        basf2.B2INFO("Found payload_boundaries: calibration strategies set to SequentialBoundaries.")
        cal.strategies = SequentialBoundaries
        for algorithm in cal.algorithms:
            algorithm.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}
    else:
        for algorithm in cal.algorithms:
            algorithm.params = {"iov_coverage": output_iov}

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal]

##############################
