# -*- coding: utf-8 -*-

"""
Airflow script for TOP post-tracking calibration:
   BS13d carrier shifts, module T0 and common T0

Author: Marko Staric, Umberto Tamponi, Shahab Kohani
"""

from prompt import CalibrationSettings, input_data_filters
from caf.utils import IoV
from caf.strategies import SequentialBoundaries
from top_calibration import BS13d_calibration_cdst
from top_calibration import moduleT0_calibration_DeltaT, moduleT0_calibration_LL
from top_calibration import commonT0_calibration_BF
from prompt.calibrations.caf_top_pre import settings as top_pretracking

#: Required variable - tells the automated system some details of this script
settings = CalibrationSettings(
    name="TOP post-tracking calibration",
    expert_username="skohani",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["bhabha_all_calib"],
    input_data_filters={
        "bhabha_all_calib": [
            input_data_filters["Data Tag"]["bhabha_all_calib"],
            input_data_filters["Run Type"]["physics"],
            input_data_filters["Data Quality Tag"]["Good Or Recoverable"]]},
    depends_on=[top_pretracking],
    expert_config={
        "max_files_per_run": 10,
        "payload_boundaries": None,
        "request_memory": "4 GB"
    })


# Required function
def get_calibrations(input_data, **kwargs):
    '''
    Returns a list of calibration objects.
    :input_data (dict): Contains every file name from the 'input_data_names' as a key.
    :**kwargs: Configuration options to be sent in.
    '''

    file_to_iov = input_data["bhabha_all_calib"]
    sample = 'bhabha'
    inputFiles = list(file_to_iov.keys())
    requested_iov = kwargs.get("requested_iov", None)
    expert_config = kwargs.get("expert_config")
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    cal = [BS13d_calibration_cdst(inputFiles),  # this is run-dep
           moduleT0_calibration_DeltaT(inputFiles),  # this cal cannot span across experiments
           moduleT0_calibration_LL(inputFiles, sample),  # this cal cannot span across experiments
           commonT0_calibration_BF(inputFiles)]  # this is run-dep

    for c in cal:
        # If it's a SequentialBoundary calibration, check if there is any boundary in the config file
        if c.strategies[0] == SequentialBoundaries:

            # Default boundaries. If there are no boundaries in the config file, this calibration will give a single IoV
            payload_boundaries = [[output_iov.exp_low, output_iov.run_low]]

            # user-defined boundaries are set here.
            if expert_config["payload_boundaries"] is not None:
                payload_boundaries = expert_config["payload_boundaries"]

            # Set the actual boundaries.
            for alg in c.algorithms:
                alg.params = {"iov_coverage": output_iov, "payload_boundaries": payload_boundaries}

        # If it's not a SequentialBoundary calbration, just set the IoV coverage
        else:
            for alg in c.algorithms:
                alg.params = {"iov_coverage": output_iov}

    # Don't save the rough moduleT0 result
    cal[1].save_payloads = False

    cal[1].depends_on(cal[0])
    cal[2].depends_on(cal[1])
    cal[3].depends_on(cal[2])

    return cal
