# -*- coding: utf-8 -*-

"""Find expected energies for ECL e+e- --> gamma gamma single crystal
energy calibration. Requires e+e- --> mu mu mc in mdst format with
ECLDigits as additional branch."""

from prompt import CalibrationSettings, input_data_filters

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(name="ecl_mu_mu_expected",
                               expert_username="hearty",
                               description=__doc__,
                               input_data_formats=["mdst"],
                               input_data_names=["mu_mu_mc"],
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
    # ..Input data
    file_to_iov_mu_mu = input_data["mu_mu_mc"]
    input_files_mu_mu = list(file_to_iov_mu_mu.keys())

    # ..Algorithm
    algo_mu_mu = Belle2.ECL.eclMuMuEAlgorithm()
    algo_mu_mu.cellIDLo = 1
    algo_mu_mu.cellIDHi = 8736
    algo_mu_mu.minEntries = 150
    algo_mu_mu.nToRebin = 1000
    algo_mu_mu.tRatioMin = 0.05
    algo_mu_mu.tRatioMax = 0.4
    algo_mu_mu.lowerEdgeThresh = 0.10
    algo_mu_mu.performFits = True
    algo_mu_mu.findExpValues = True
    algo_mu_mu.storeConst = 0

    # ..The calibration
    eclmumu_collector = basf2.register_module("eclMuMuECollector")
    eclmumu_collector.param("granularity", "all")
    eclmumu_collector.param("minPairMass", 9.0)
    eclmumu_collector.param("minTrackLength", 30.)
    eclmumu_collector.param("MaxNeighbourE", 0.010)
    eclmumu_collector.param("thetaLabMinDeg", 17.)
    eclmumu_collector.param("thetaLabMaxDeg", 150.)
    eclmumu_collector.param("measureTrueEnergy", True)
    eclmumu_collector.param("requireL1", False)
    cal_ecl_mu_mu = Calibration(
        name="ecl_mu_mu_expected",
        collector=eclmumu_collector,
        algorithms=algo_mu_mu,
        input_files=input_files_mu_mu)

    # ..Need to include track extrapolation in the path before collector
    ext_path = basf2.create_path()
    ext_path.add_module("Gearbox")
    ext_path.add_module("Geometry")
    ext_path.add_module("Ext", pdgCodes=[13])
    cal_ecl_mu_mu.pre_collector_path = ext_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_mu_mu.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_mu_mu]
