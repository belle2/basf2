# -*- coding: utf-8 -*-

"""
airflow script for PXD hot/dead pixel masking.
Author: qingyuan.liu@desy.de
"""

import basf2
from pxd.calibration import hot_pixel_mask_calibration
from prompt.utils import filter_by_max_files_per_run
from prompt import CalibrationSettings
from caf.utils import IoV

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="PXD hot/dead pixel calibration",
                               expert_username="qyliu",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["beamorphyscs", "cosmic"],
                               depends_on=[])


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["output_iov"] is used. This is the output IoV range that your payloads should
        correspond to. Generally your highest ExpRun payload should be open ended e.g. IoV(3,4,-1,-1)

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """

    # Set up config options
    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # Read input_data
    file_to_iov_physics = input_data["beamorphyscs"]
    file_to_iov_cosmics = input_data["cosmic"]

    # Reduce data and create calibration instances for different data categories
    cal_list = []
    max_files_per_run = 20
    if len(file_to_iov_physics) > 0:
        reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
        input_files_physics = list(reduced_file_to_iov_physics.keys())
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_physics)}")
        cal_list.append(hot_pixel_mask_calibration(input_files_physics))
    if len(file_to_iov_cosmics) > 0:
        reduced_file_to_iov_cosmics = filter_by_max_files_per_run(file_to_iov_cosmics, max_files_per_run)
        input_files_cosmics = list(reduced_file_to_iov_cosmics.keys())
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_cosmics)}")
        # cal_list.append(hot_pixel_mask_calibration(input_files_cosmics, run_type='cosmic'))
        cal = hot_pixel_mask_calibration(input_files_cosmics, run_type='cosmic')
        cal.name += 'Cosmic'
        cal_list.append(cal)

    # Leave the last iov open
    for cal in cal_list:
        for algorithm in cal.algorithms:
            algorithm.params = {"iov_coverage": output_iov}

    return cal_list
