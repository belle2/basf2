# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""ECL Autocovariance."""

from prompt import CalibrationSettings, INPUT_DATA_FILTERS

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(
    name="ecl_autocovariance",
    expert_username="longos",
    description=__doc__,
    input_data_formats=["raw"],
    input_data_names=["delayedbhabha"],
    input_data_filters={
        "delayedbhabha": [
            INPUT_DATA_FILTERS["Data Tag"]["delayedbhabha"],
            INPUT_DATA_FILTERS["Data Quality Tag"]["Good Or Recoverable"],
            INPUT_DATA_FILTERS["Run Type"]["physics"],
            INPUT_DATA_FILTERS["Magnet"]["On"]]},
    depends_on=[],
    expert_config={
        "C1_lowestEnergyFraction": 0.75,
        "C1_TotalCountsThreshold": 10000
    })


# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration

    # --------------------------------------------------------------
    # ..delayed Bhabha

    # ..Input data
    file_to_iov_delayed_Bhabha = input_data["delayedbhabha"]
    input_files = list(file_to_iov_delayed_Bhabha.keys())

    # ..Algorithm
    algo_C1 = Belle2.ECL.eclAutocovarianceCalibrationC1Algorithm()
    expert_config = kwargs.get("expert_config")
    algo_C1.setLowestEnergyFraction(expert_config["C1_lowestEnergyFraction"])
    algo_C1.setTotalCountsThreshold(expert_config["C1_TotalCountsThreshold"])

    # ..The calibration
    collector_C1 = basf2.register_module("eclAutocovarianceCalibrationC1Collector")

    cal_ecl_Auto_C1 = Calibration("ecl_Auto_C1",
                                  collector=collector_C1,
                                  algorithms=[algo_C1],
                                  input_files=input_files, max_files_per_collector_job=4)

    # ..Add prepare_cdst_analysis to pre_collector_path
    delayed_Bhabha_pre_path = basf2.create_path()
    delayed_Bhabha_pre_path.add_module("RootInput", inputFileNames="", branchNames=["EventMetaData", "RawECLs"])
    eclunpacker = basf2.register_module('ECLUnpacker')
    delayed_Bhabha_pre_path.add_module(eclunpacker)

    cal_ecl_Auto_C1.pre_collector_path = delayed_Bhabha_pre_path

    # ..Algorithm
    algo_C2 = Belle2.ECL.eclAutocovarianceCalibrationC2Algorithm()

    # ..The calibration
    collector_C2 = basf2.register_module("eclAutocovarianceCalibrationC2Collector")

    cal_ecl_Auto_C2 = Calibration("ecl_Auto_C2",
                                  collector=collector_C2,
                                  algorithms=[algo_C2],
                                  input_files=input_files, max_files_per_collector_job=4)

    cal_ecl_Auto_C2.pre_collector_path = delayed_Bhabha_pre_path

    cal_ecl_Auto_C2.depends_on(cal_ecl_Auto_C1)

    # ..Algorithm
    algo_C3 = Belle2.ECL.eclAutocovarianceCalibrationC3Algorithm()

    # ..The calibration
    collector_C3 = basf2.register_module("eclAutocovarianceCalibrationC3Collector")

    cal_ecl_Auto_C3 = Calibration("ecl_Auto_C3",
                                  collector=collector_C3,
                                  algorithms=[algo_C3],
                                  input_files=input_files, max_files_per_collector_job=4)

    cal_ecl_Auto_C3.pre_collector_path = delayed_Bhabha_pre_path

    cal_ecl_Auto_C3.depends_on(cal_ecl_Auto_C2)

    # ..Algorithm
    algo_C4 = Belle2.ECL.eclAutocovarianceCalibrationC4Algorithm()

    # ..The calibration
    collector_C4 = basf2.register_module("eclAutocovarianceCalibrationC4Collector")

    cal_ecl_Auto_C4 = Calibration("ecl_Auto_C4",
                                  collector=collector_C4,
                                  algorithms=[algo_C4],
                                  input_files=input_files, max_files_per_collector_job=4)

    cal_ecl_Auto_C4.pre_collector_path = delayed_Bhabha_pre_path

    cal_ecl_Auto_C4.depends_on(cal_ecl_Auto_C3)

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_Auto_C1.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_Auto_C2.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_Auto_C3.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_Auto_C4.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_Auto_C1, cal_ecl_Auto_C2, cal_ecl_Auto_C3, cal_ecl_Auto_C4]
