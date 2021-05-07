# -*- coding: utf-8 -*-

"""
Airflow script for TOP pre-tracking calibration:
   channel masks

Author: Marko Staric
"""

from prompt import CalibrationSettings, input_data_filters
from caf.utils import IoV
from caf.strategies import SequentialBoundaries
from top_calibration import channel_mask_calibration

#: Required variable - tells the automated system some details of this script
settings = CalibrationSettings(
    name="TOP pre-tracking calibration",
    expert_username="skohani",
    description=__doc__,
    input_data_formats=["raw"],
    input_data_names=["hadron_calib"],
    input_data_filters={
        "hadron_calib": [
            input_data_filters["Data Tag"]["hadron_calib"],
            input_data_filters["Run Type"]["physics"],
            input_data_filters["Data Quality Tag"]["Good Or Recoverable"]]},
    depends_on=[],
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

    file_to_iov = input_data["hadron_calib"]
    inputFiles = list(file_to_iov.keys())
    requested_iov = kwargs.get("requested_iov", None)
    expert_config = kwargs.get("expert_config")
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    cal = [channel_mask_calibration(inputFiles)]  # this is run-dep

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

    return cal
