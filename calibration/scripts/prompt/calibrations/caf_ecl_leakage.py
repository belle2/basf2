# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""ECL calibration to calculate photon energy leakage corrections."""

from prompt import CalibrationSettings, input_data_filters


# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(name="ecl_leakage",
                               expert_username="hearty",
                               description=__doc__,
                               input_data_formats=["mdst"],
                               input_data_names=["single_gamma_mc"],
                               input_data_filters={
                                   "single_gamma_mc": [
                                       input_data_filters["Data Tag"]["single_gamma_mc"]
                                   ]
                               },
                               depends_on=[],
                               expert_config={"number_energies": 8,
                                              "forward_energies": [0.030, 0.050, 0.100, 0.200, 0.483, 1.166, 2.816, 6.800],
                                              "barrel_energies": [0.030, 0.050, 0.100, 0.200, 0.458, 1.049, 2.402, 5.500],
                                              "backward_energies": [0.030, 0.050, 0.100, 0.200, 0.428, 0.917, 1.962, 4.200]}
                               )


# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration

    # --------------------------------------------------------------
    # ..Input data
    file_to_iov_leakage = input_data["single_gamma_mc"]
    input_files_leakage = list(file_to_iov_leakage.keys())

    # ..Algorithm
    algo_leakage = Belle2.ECL.eclLeakageAlgorithm()

    # ..Collector
    ecl_leakage_collector = basf2.register_module("eclLeakageCollector")
    ecl_leakage_collector.param("granularity", "all")
    ecl_leakage_collector.param("position_bins", 29)

    # ..Number of energy points and their values can be set via expert_config
    expert_config = kwargs.get("expert_config")
    number_energies = expert_config["number_energies"]
    ecl_leakage_collector.param("number_energies", number_energies)

    forward_energies = expert_config["forward_energies"]
    ecl_leakage_collector.param("energies_forward", forward_energies)

    barrel_energies = expert_config["barrel_energies"]
    ecl_leakage_collector.param("energies_barrel", barrel_energies)

    backward_energies = expert_config["backward_energies"]
    ecl_leakage_collector.param("energies_backward", backward_energies)

    # ..The calibration
    cal_ecl_leakage = Calibration(
        name="ecl_leakage",
        collector=ecl_leakage_collector,
        algorithms=algo_leakage,
        input_files=input_files_leakage)

    # ..pre_path is empty
    ecl_leakage_pre_path = basf2.create_path()
    cal_ecl_leakage.pre_collector_path = ecl_leakage_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_leakage.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_leakage]
