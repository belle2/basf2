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
from itertools import groupby
from itertools import chain

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
    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    reduced_file_to_iov_cosmics = filter_by_max_files_per_run(file_to_iov_cosmics, max_files_per_run)

    # Create run chunks based on exp no. and run type
    iov_set_physics = set(reduced_file_to_iov_physics.values())
    iov_set_cosmics = set(reduced_file_to_iov_cosmics.values())

    iov_list_cosmics = list(sorted(iov_set_cosmics))
    # iov_list_physics = list(sorted(iov_set_physics))
    iov_list_all = list(sorted(iov_set_cosmics | iov_set_physics))

    exp_set = set([iov.exp_low for iov in iov_list_all])
    chunks_exp = []
    for exp in sorted(exp_set):
        chunks_exp += [list(g) for k, g in groupby(iov_list_all, lambda x: x.exp_low == exp) if k]

    chunks_phy = []
    chunks_cosmic = []
    for chunk_exp in chunks_exp:
        chunks_phy += [list(g) for k, g in groupby(chunk_exp, lambda x: x in iov_list_cosmics) if not k]
        chunks_cosmic += [list(g) for k, g in groupby(chunk_exp, lambda x: x in iov_list_cosmics) if k]

    # Create calibrations

    # Physics or beam run
    chunk_list = chunks_phy
    input_data = reduced_file_to_iov_physics
    iSkip = -1
    for ichunk, chunk in enumerate(chunk_list):
        first_iov = IoV(chunk[0].exp_low, chunk[0].run_low, -1, -1)
        last_iov = IoV(chunk[-1].exp_low, chunk[-1].run_low, -1, -1)
        if last_iov < output_iov:  # All the chunk iovs are earlier than the requested
            iSkip = ichunk
            continue
        else:
            # From the second chunk within the requested range, we have the iov defined by the first run
            specific_iov = first_iov if ichunk > (iSkip + 1) else output_iov
        input_files = list(chain.from_iterable([list(g) for k, g in groupby(
            input_data, lambda x: input_data[x] in chunk) if k]))
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files)} for the output {specific_iov}")
        cal = hot_pixel_mask_calibration(
                cal_name="{}_PXDHotPixelMaskCalibration_BeamorPhysics".format(ichunk+1),
                input_files=input_files)
        cal.algorithms[0].params = {"iov_coverage": specific_iov}
        cal_list.append(cal)

    # Cosmic run
    nchunks_phy = len(chunks_phy)
    chunk_list = chunks_cosmic
    input_data = reduced_file_to_iov_cosmics
    iSkip = -1
    for ichunk, chunk in enumerate(chunk_list):
        first_iov = IoV(chunk[0].exp_low, chunk[0].run_low, chunk[-1].exp_high, chunk[-1].run_high)
        last_iov = IoV(chunk[-1].exp_low, chunk[-1].run_low, chunk[-1].exp_high, chunk[-1].run_high)
        if last_iov < output_iov:  # All the chunk iovs are earlier than the requested
            iSkip = ichunk
            continue
        else:
            # From the first chunk within the requested range, we have the iov defined by the first run
            if ichunk == (iSkip + 1):
                specific_iov = max(first_iov, IoV(
                    requested_iov.exp_low, requested_iov.run_low, chunk[-1].exp_high, chunk[-1].run_high))
            else:  # ichunk > (iSkip + 1)
                specific_iov = first_iov
        input_files = list(chain.from_iterable([list(g) for k, g in groupby(
            input_data, lambda x: input_data[x] in chunk) if k]))
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files)} for the output {specific_iov}")
        cal = hot_pixel_mask_calibration(
                cal_name="{}_PXDHotPixelMaskCalibration_Cosmic".format(ichunk+1+nchunks_phy),
                input_files=input_files,
                run_type='cosmic')
        cal.algorithms[0].params = {"iov_coverage": specific_iov}
        cal_list.append(cal)

    return cal_list
    # return []
