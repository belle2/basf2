##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""ECL cluster crystal energy calibration using single photon MCrd."""

from prompt import CalibrationSettings, INPUT_DATA_FILTERS

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(
    name="ecl_cluster_energy",
    expert_username="hearty",
    description=__doc__,
    input_data_formats=["mdst"],
    input_data_names=["single_gamma_mc"],
    input_data_filters={"single_gamma_mc": [INPUT_DATA_FILTERS["Data Tag"]["single_gamma_mc"]]},
    depends_on=[],

    # ..The expert_config settings should agree with the parameters of the jobs used
    #   to create the single_gamma_mc samples.
    #   Number of energy points, and the values of these for each of the three ECL regions.
    #   The mc samples include a reduced set of showers and CalDigits, with these names.
    expert_config={"number_energies": 8,
                   "forward_energies": [0.030, 0.050, 0.100, 0.200, 0.483, 1.166, 2.816, 6.800],
                   "barrel_energies": [0.030, 0.050, 0.100, 0.200, 0.458, 1.049, 2.402, 5.500],
                   "backward_energies": [0.030, 0.050, 0.100, 0.200, 0.428, 0.917, 1.962, 4.200],
                   "digitArrayName": "ECLTrimmedDigits",
                   "showerArrayName": "ECLTrimmedShowers",
                   "nGroupPerThetaID": 8,
                   "lowEnergyThreshold": 0.}
                               )

# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration

    # --------------------------------------------------------------
    # ..Input data, same for both calibrations
    file_to_iov = input_data["single_gamma_mc"]
    input_files_single_gamma = list(file_to_iov.keys())

    # --------------------------------------------------------------
    # ..nOptimal calibration first

    # ..eclNOptimal collector
    ecl_nOptimal_collector = basf2.register_module("eclNOptimalCollector")
    ecl_nOptimal_collector.param("granularity", "all")

    # ..Number of energy points and their values can be set via expert_config
    expert_config = kwargs.get("expert_config")
    number_energies = expert_config["number_energies"]
    ecl_nOptimal_collector.param("numberEnergies", number_energies)

    forward_energies = expert_config["forward_energies"]
    ecl_nOptimal_collector.param("energiesForward", forward_energies)

    barrel_energies = expert_config["barrel_energies"]
    ecl_nOptimal_collector.param("energiesBarrel", barrel_energies)

    backward_energies = expert_config["backward_energies"]
    ecl_nOptimal_collector.param("energiesBackward", backward_energies)

    digitArrayName = expert_config["digitArrayName"]
    ecl_nOptimal_collector.param("digitArrayName", digitArrayName)

    showerArrayName = expert_config["showerArrayName"]
    ecl_nOptimal_collector.param("showerArrayName", showerArrayName)

    nGroupPerThetaID = expert_config["nGroupPerThetaID"]
    ecl_nOptimal_collector.param("nGroupPerThetaID", nGroupPerThetaID)

    # ..eclNOptimal algorithm
    algo_nOptimal = Belle2.ECL.eclNOptimalAlgorithm()

    # ..The calibration
    cal_ecl_nOptimal = Calibration(
        name="ecl_nOptimal",
        collector=ecl_nOptimal_collector,
        algorithms=algo_nOptimal,
        input_files=input_files_single_gamma)

    # ..pre_path is empty
    ecl_nOptimal_pre_path = basf2.create_path()
    cal_ecl_nOptimal.pre_collector_path = ecl_nOptimal_pre_path

    # --------------------------------------------------------------
    # ..Leakage correction, which depends on nOptimal

    # ..eclLeakage collector
    ecl_leakage_collector = basf2.register_module("eclLeakageCollector")
    ecl_leakage_collector.param("granularity", "all")
    ecl_leakage_collector.param("position_bins", 29)

    # ..Number of energy points and their values can be set via expert_config
    ecl_leakage_collector.param("number_energies", number_energies)
    ecl_leakage_collector.param("energies_forward", forward_energies)
    ecl_leakage_collector.param("energies_barrel", barrel_energies)
    ecl_leakage_collector.param("energies_backward", backward_energies)
    ecl_leakage_collector.param("showerArrayName", showerArrayName)

    # ..eclLeakage algorithm
    algo_leakage = Belle2.ECL.eclLeakageAlgorithm()
    lowEnergyThreshold = expert_config["lowEnergyThreshold"]
    algo_leakage.setLowEnergyThreshold(lowEnergyThreshold)

    # ..The calibration
    cal_ecl_leakage = Calibration(
        name="ecl_leakage",
        collector=ecl_leakage_collector,
        algorithms=algo_leakage,
        input_files=input_files_single_gamma)

    # ..Depends on nOptimal
    cal_ecl_leakage.depends_on(cal_ecl_nOptimal)

    # ..pre_path is empty
    ecl_leakage_pre_path = basf2.create_path()
    cal_ecl_leakage.pre_collector_path = ecl_leakage_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_nOptimal.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal_ecl_leakage.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_nOptimal, cal_ecl_leakage]
