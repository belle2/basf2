# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""ECL waveform template calibration"""

from prompt import CalibrationSettings, INPUT_DATA_FILTERS

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(
    name="ecl_WaveformTemplateShapeCalibration",
    expert_username="longos",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["gamma_gamma_calib"],
    input_data_filters={
        "gamma_gamma_calib": [
            INPUT_DATA_FILTERS["Data Tag"]["gamma_gamma_calib"],
            INPUT_DATA_FILTERS["Data Quality Tag"]["Good Or Recoverable"],
            INPUT_DATA_FILTERS["Run Type"]["physics"],
            INPUT_DATA_FILTERS["Magnet"]["On"]]},
    depends_on=[],
    expert_config={})


# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration

    # --------------------------------------------------------------
    # ..gamma gamma

    # ..Input data
    file_to_iov_gamma_gamma = input_data["gamma_gamma_calib"]
    input_files = list(file_to_iov_gamma_gamma.keys())

    # ..Algorithm
    algo_C1 = Belle2.ECL.eclWaveformTemplateCalibrationC1Algorithm()

    # ..The calibration
    collector_C1 = basf2.register_module("eclWaveformTemplateCalibrationC1Collector")

    cal_ecl_Wave_C1 = Calibration(
        "ecl_Wave_C1",
        collector=collector_C1,
        algorithms=[algo_C1],
        input_files=input_files,
        max_files_per_collector_job=4)

    # ..Add prepare_cdst_analysis to pre_collector_path
    gamma_gamma_pre_path = basf2.create_path()
    gamma_gamma_pre_path.add_module("RootInput", inputFileNames="", branchNames=["EventMetaData", "RawECLs"])
    eclunpacker = basf2.register_module('ECLUnpacker')
    gamma_gamma_pre_path.add_module(eclunpacker)

    cal_ecl_Wave_C1.pre_collector_path = gamma_gamma_pre_path

    LowLimit = 1
    UpperLimit = 8736

    # ..Algorithm
    algos_C2 = Belle2.ECL.eclWaveformTemplateCalibrationC2Algorithm()
    algos_C2.setFirstCellID(LowLimit)
    algos_C2.setLastCellID(UpperLimit)

    algos_C3 = Belle2.ECL.eclWaveformTemplateCalibrationC3Algorithm()
    algos_C3.setFirstCellID(LowLimit)
    algos_C3.setLastCellID(UpperLimit)

    collectors_C2 = basf2.register_module("eclWaveformTemplateCalibrationC2Collector")
    collectors_C2.pre_collector_path = gamma_gamma_pre_path
    collectors_C2.param('MinCellID', LowLimit)
    collectors_C2.param('MaxCellID', UpperLimit)

    calibrations_C2 = Calibration(
        "ecl_Wave_C2_" +
        str(LowLimit) +
        "_" +
        str(UpperLimit),
        collector=collectors_C2,
        algorithms=[
            algos_C2,
            algos_C3],
        input_files=input_files,
        max_files_per_collector_job=4)
    calibrations_C2.pre_collector_path = gamma_gamma_pre_path
    calibrations_C2.depends_on(cal_ecl_Wave_C1)

    # ..Algorithm
    algo_C4 = Belle2.ECL.eclWaveformTemplateCalibrationC4Algorithm()
    algo_C4.setFirstCellID(LowLimit)
    algo_C4.setLastCellID(UpperLimit)

    # ..The calibration
    cal_ecl_Wave_C4 = Calibration("ecl_Wave_C4",
                                  collector="DummyCollector",
                                  algorithms=[algo_C4],
                                  input_files=input_files[:1],
                                  )

    cal_ecl_Wave_C4.depends_on(cal_ecl_Wave_C1)
    cal_ecl_Wave_C4.depends_on(calibrations_C2)

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_Wave_C1.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in calibrations_C2.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_Wave_C4.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    calList = [cal_ecl_Wave_C1, calibrations_C2, cal_ecl_Wave_C4]
    return calList
