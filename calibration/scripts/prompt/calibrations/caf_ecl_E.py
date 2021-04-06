# -*- coding: utf-8 -*-

"""ECL single crystal energy calibration using three control samples."""

from prompt import CalibrationSettings, input_data_filters

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(
    name="ecl_energy",
    expert_username="hearty",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=[
        "bhabha_all_calib",
        "gamma_gamma_calib",
        "mumutight_calib"],
    input_data_filters={
        "bhabha_all_calib": [
            input_data_filters["Data Tag"]["bhabha_all_calib"],
            input_data_filters["Data Quality Tag"]["Good"],
            input_data_filters["Magnet"]["On"]],
        "gamma_gamma_calib": [
            input_data_filters["Data Tag"]["gamma_gamma_calib"],
            input_data_filters["Data Quality Tag"]["Good"],
            input_data_filters["Magnet"]["On"]],
        "mumutight_calib": [
            input_data_filters["Data Tag"]["mumutight_calib"],
            input_data_filters["Data Quality Tag"]["Good Or Recoverable"],
            input_data_filters["Magnet"]["On"]]},
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
    # ..Bhabha

    # ..Input data
    file_to_iov_bhabha = input_data["bhabha_all_calib"]
    input_files_bhabha = list(file_to_iov_bhabha.keys())

    # ..Algorithm
    algo_ee5x5 = Belle2.ECL.eclee5x5Algorithm()
    algo_ee5x5.setMinEntries(150)
    algo_ee5x5.setPayloadName("ECLCrystalEnergy5x5")
    algo_ee5x5.setStoreConst(True)

    # ..The calibration
    eclee5x5_collector = basf2.register_module("eclee5x5Collector")
    eclee5x5_collector.param("thetaLabMinDeg", 17.)
    eclee5x5_collector.param("thetaLabMaxDeg", 150.)
    eclee5x5_collector.param("minE0", 0.45)
    eclee5x5_collector.param("minE1", 0.40)
    eclee5x5_collector.param("maxdThetaSum", 2.)
    eclee5x5_collector.param("dPhiScale", 1.)
    eclee5x5_collector.param("maxTime", 10.)
    eclee5x5_collector.param("useCalDigits", False)
    eclee5x5_collector.param("requireL1", False)
    eclee5x5_collector.param("granularity", "all")
    cal_ecl_ee5x5 = Calibration("ecl_ee5x5",
                                collector=eclee5x5_collector,
                                algorithms=[algo_ee5x5],
                                input_files=input_files_bhabha
                                )
    cal_ecl_ee5x5.backend_args = {"request_memory": "4 GB"}

    # ..Add prepare_cdst_analysis to pre_collector_path
    ee5x5_pre_path = basf2.create_path()
    prepare_cdst_analysis(ee5x5_pre_path, components=['ECL'])
    cal_ecl_ee5x5.pre_collector_path = ee5x5_pre_path

    # --------------------------------------------------------------
    # ..gamma gamma

    # ..Input data
    file_to_iov_gamma_gamma = input_data["gamma_gamma_calib"]
    input_files_gamma_gamma = list(file_to_iov_gamma_gamma.keys())

    # ..Algorithm
    algo_gamma_gamma = Belle2.ECL.eclGammaGammaEAlgorithm()
    algo_gamma_gamma.setOutputName("eclGammaGammaE_algorithm.root")
    algo_gamma_gamma.setCellIDLo(1)
    algo_gamma_gamma.setCellIDHi(8736)
    algo_gamma_gamma.setMinEntries(150)
    algo_gamma_gamma.setMaxIterations(10)
    algo_gamma_gamma.setTRatioMin(0.45)
    algo_gamma_gamma.setTRatioMax(0.60)
    algo_gamma_gamma.setUpperEdgeThresh(0.02)
    algo_gamma_gamma.setPerformFits(True)
    algo_gamma_gamma.setFindExpValues(False)
    algo_gamma_gamma.setStoreConst(0)

    # ..The calibration
    eclGammaGamma_collector = basf2.register_module("eclGammaGammaECollector")
    eclGammaGamma_collector.param("granularity", "all")
    eclGammaGamma_collector.param("thetaLabMinDeg", 0.)
    eclGammaGamma_collector.param("thetaLabMaxDeg", 180.)
    eclGammaGamma_collector.param("minPairMass", 9.)
    eclGammaGamma_collector.param("mindPhi", 179.)
    eclGammaGamma_collector.param("maxTime", 999.)
    eclGammaGamma_collector.param("measureTrueEnergy", False)
    eclGammaGamma_collector.param("requireL1", False)
    cal_ecl_gamma_gamma = Calibration("ecl_gamma_gamma",
                                      collector=eclGammaGamma_collector,
                                      algorithms=[algo_gamma_gamma],
                                      input_files=input_files_gamma_gamma
                                      )

    # ..Add prepare_cdst_analysis to pre_collector_path
    gamma_gamma_pre_path = basf2.create_path()
    prepare_cdst_analysis(gamma_gamma_pre_path, components=['ECL'])
    cal_ecl_gamma_gamma.pre_collector_path = gamma_gamma_pre_path

    # --------------------------------------------------------------
    # ..muon pair

    # ..Input data
    file_to_iov_mu_mu = input_data["mumutight_calib"]
    input_files_mu_mu = list(file_to_iov_mu_mu.keys())

    # ..Algorithm
    algo_mu_mu = Belle2.ECL.eclMuMuEAlgorithm()
    algo_mu_mu.cellIDLo = 1
    algo_mu_mu.cellIDHi = 8736
    algo_mu_mu.minEntries = 150
    algo_mu_mu.maxIterations = 10
    algo_mu_mu.tRatioMin = 0.2
    algo_mu_mu.tRatioMax = 0.25
    algo_mu_mu.performFits = True
    algo_mu_mu.findExpValues = False
    algo_mu_mu.storeConst = 0

    # ..The calibration
    eclmumu_collector = basf2.register_module("eclMuMuECollector")
    eclmumu_collector.param("granularity", "all")
    eclmumu_collector.param("minPairMass", 9.0)
    eclmumu_collector.param("minTrackLength", 30.)
    eclmumu_collector.param("MaxNeighbourE", 0.010)
    eclmumu_collector.param("thetaLabMinDeg", 17.)
    eclmumu_collector.param("thetaLabMaxDeg", 150.)
    eclmumu_collector.param("measureTrueEnergy", False)
    eclmumu_collector.param("requireL1", False)
    cal_ecl_mu_mu = Calibration(name="ecl_mu_mu", collector=eclmumu_collector, algorithms=algo_mu_mu, input_files=input_files_mu_mu)

    # ..Need to include track extrapolation in the path before collector
    ext_path = basf2.create_path()
    prepare_cdst_analysis(ext_path, components=['ECL'])
    ext_path.add_module("Ext", pdgCodes=[13])
    cal_ecl_mu_mu.pre_collector_path = ext_path

    # --------------------------------------------------------------
    # Include a merging Calibration that doesn't require input data but instead creates the final
    # payload from the previous calibration payloads.

    # We use a dummy collector that barely outputs any data and we set the input files to a single file so
    # we spawn only one very fast job.
    # It doesn't matter which input file we choose as the output is never used.

    merging_alg = Belle2.ECL.eclMergingCrystalEAlgorithm()
    cal_ecl_merge = Calibration(name="ecl_merge", collector="DummyCollector", algorithms=[merging_alg],
                                input_files=input_files_mu_mu[:1])

    # The important part is that we depend on all 3 previous calibrations
    cal_ecl_merge.depends_on(cal_ecl_ee5x5)
    cal_ecl_merge.depends_on(cal_ecl_gamma_gamma)
    cal_ecl_merge.depends_on(cal_ecl_mu_mu)

    # ..Uses cdst data so it requires prepare_cdst_analysis
    ecl_merge_pre_path = basf2.create_path()
    prepare_cdst_analysis(ecl_merge_pre_path, components=['ECL'])
    ecl_merge_pre_path.pre_collector_path = ecl_merge_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_ee5x5.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_gamma_gamma.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_mu_mu.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_merge.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_ee5x5, cal_ecl_gamma_gamma, cal_ecl_mu_mu, cal_ecl_merge]
