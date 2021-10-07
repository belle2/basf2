# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Find expected energies for ECL e+e- --> gamma gamma single crystal
energy calibration. Requires e+e- --> gamma gamma mc in mdst format."""

from prompt import CalibrationSettings, input_data_filters

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(
    name="ecl_gg_expected",
    expert_username="hearty",
    description=__doc__,
    input_data_formats=["mdst"],
    input_data_names=["gamma_gamma_mc"],
    depends_on=[])

# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration
    from reconstruction import prepare_cdst_analysis

    # --------------------------------------------------------------
    # ..gamma gamma

    # ..Input data
    file_to_iov_gamma_gamma = input_data["gamma_gamma_mc"]
    input_files_gamma_gamma = list(file_to_iov_gamma_gamma.keys())

    # ..Algorithm
    algo_gamma_gamma = Belle2.ECL.eclGammaGammaEAlgorithm()
    algo_gamma_gamma.setOutputName("eclGammaGammaE_algorithm.root")
    algo_gamma_gamma.setCellIDLo(1)
    algo_gamma_gamma.setCellIDHi(8736)
    algo_gamma_gamma.setMinEntries(150)
    algo_gamma_gamma.setMaxIterations(10)
    algo_gamma_gamma.setTRatioMin(0.45)
    algo_gamma_gamma.setTRatioMax(0.70)
    algo_gamma_gamma.setTRatioMinHiStat(0.70)
    algo_gamma_gamma.setTRatioMaxHiStat(0.95)
    algo_gamma_gamma.setUpperEdgeThresh(0.02)
    algo_gamma_gamma.setPerformFits(True)
    algo_gamma_gamma.setFindExpValues(True)
    algo_gamma_gamma.setStoreConst(0)

    # ..The calibration
    eclGammaGamma_collector = basf2.register_module("eclGammaGammaECollector")
    eclGammaGamma_collector.param("granularity", "all")
    eclGammaGamma_collector.param("thetaLabMinDeg", 0.)
    eclGammaGamma_collector.param("thetaLabMaxDeg", 180.)
    eclGammaGamma_collector.param("minPairMass", 9.)
    eclGammaGamma_collector.param("mindPhi", 179.)
    eclGammaGamma_collector.param("maxTime", 999.)
    eclGammaGamma_collector.param("measureTrueEnergy", True)
    eclGammaGamma_collector.param("requireL1", False)
    cal_ecl_gamma_gamma = Calibration("ecl_gg_expected",
                                      collector=eclGammaGamma_collector,
                                      algorithms=[algo_gamma_gamma],
                                      input_files=input_files_gamma_gamma
                                      )

    # ..Add prepare_cdst_analysis to pre_collector_path
    gamma_gamma_pre_path = basf2.create_path()
    cal_ecl_gamma_gamma.pre_collector_path = gamma_gamma_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_gamma_gamma.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_gamma_gamma]
