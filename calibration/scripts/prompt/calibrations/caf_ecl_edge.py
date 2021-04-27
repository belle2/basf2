# -*- coding: utf-8 -*-

"""ECL calibration to specify edges of each crystal."""

from prompt import CalibrationSettings, input_data_filters

# --------------------------------------------------------------
# ..Tell the automated script some required details
settings = CalibrationSettings(name="ecl_edge",
                               expert_username="hearty",
                               description=__doc__,
                               input_data_formats=["mdst"],
                               input_data_names=["mumutight_calib"],
                               input_data_filters={"mumutight_calib": [input_data_filters["Data Tag"]["mumutight_calib"],
                                                                       input_data_filters["Data Quality Tag"]["Good"]]},
                               depends_on=[])

# --------------------------------------------------------------
# ..The calibration functions


def get_calibrations(input_data, **kwargs):
    import basf2
    from ROOT import Belle2
    from caf.utils import IoV
    from caf.framework import Calibration

    # --------------------------------------------------------------
    # ..Input data
    file_to_iov_mu_mu = input_data["mumutight_calib"]
    input_files_mu_mu = list(file_to_iov_mu_mu.keys())

    # ..Algorithm
    algo_edge = Belle2.ECL.eclEdgeAlgorithm()

    # ..The calibration
    ecledge_collector = basf2.register_module("eclEdgeCollector")
    cal_ecl_edge = Calibration(name="ecl_edge", collector=ecledge_collector, algorithms=algo_edge,
                               input_files=input_files_mu_mu[:1], max_collector_jobs=1)

    # ..pre_path to include geometry
    ecl_edge_pre_path = basf2.create_path()
    ecl_edge_pre_path.add_module("Gearbox")
    ecl_edge_pre_path.add_module("Geometry")
    cal_ecl_edge.pre_collector_path = ecl_edge_pre_path

    # --------------------------------------------------------------
    # ..Force the output iovs to be open
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    for algorithm in cal_ecl_edge.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # --------------------------------------------------------------
    # ..Return the calibrations
    return [cal_ecl_edge]
