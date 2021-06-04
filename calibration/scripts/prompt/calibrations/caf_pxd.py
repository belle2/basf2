# -*- coding: utf-8 -*-

"""
airflow script for PXD hot/dead pixel masking.
Author: qingyuan.liu@desy.de
"""

import basf2
from pxd.calibration import hot_pixel_mask_calibration
from prompt.utils import filter_by_max_files_per_run, filter_by_max_events_per_run
from prompt import CalibrationSettings, input_data_filters
from caf.utils import IoV
from itertools import groupby
from itertools import chain
from math import ceil

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="PXD hot/dead pixel calibration",
                               expert_username="qyliu",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["beamorphysics", "cosmic"],
                               input_data_filters={"beamorphysics": [input_data_filters["Data Tag"]["bhabha_all_calib"],
                                                                     input_data_filters["Data Tag"]["gamma_gamma_calib"],
                                                                     input_data_filters["Data Tag"]["hadron_calib"],
                                                                     input_data_filters["Data Tag"]["offip_calib"],
                                                                     input_data_filters["Data Tag"]["cosmic_calib"],
                                                                     input_data_filters["Beam Energy"]["4S"],
                                                                     input_data_filters["Beam Energy"]["Continuum"],
                                                                     input_data_filters["Beam Energy"]["Scan"],
                                                                     input_data_filters["Run Type"]["physics"],
                                                                     input_data_filters["Data Quality Tag"]["Good"]],
                                                   "cosmic": [input_data_filters["Run Type"]["cosmic"]]},
                               expert_config={
                                   "max_events_per_run": 400000,
                                   "max_files_per_run": 20,  # only valid when max_events/run <= 0
                                   },
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
    expert_config = kwargs.get("expert_config")
    cal_kwargs = expert_config.get("kwargs", {})
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    expert_config = kwargs.get("expert_config")
    max_events_per_run = expert_config["max_events_per_run"]
    max_files_per_run = expert_config["max_files_per_run"]
    min_files_per_chunk = 10
    min_events_per_file = 1000  # avoid empty files

    # Read input_data
    file_to_iov_physics = input_data["beamorphysics"]
    file_to_iov_cosmics = input_data["cosmic"]

    # Reduce data and create calibration instances for different data categories
    cal_list = []
    if max_events_per_run <= 0:
        basf2.B2INFO(f"Reducing to a maximum of {max_files_per_run} files per run.")
        reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics,
                                                                  max_files_per_run, min_events_per_file)
        reduced_file_to_iov_cosmics = filter_by_max_files_per_run(file_to_iov_cosmics,
                                                                  max_files_per_run, min_events_per_file)
    else:
        basf2.B2INFO(f"Reducing to a maximum of {max_events_per_run} events per run.")
        reduced_file_to_iov_physics = filter_by_max_events_per_run(file_to_iov_physics,
                                                                   max_events_per_run, random_select=True)
        reduced_file_to_iov_cosmics = filter_by_max_events_per_run(file_to_iov_cosmics,
                                                                   max_events_per_run, random_select=True)

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
    iCal = 0
    for ichunk, chunk in enumerate(chunk_list):
        first_iov = IoV(chunk[0].exp_low, chunk[0].run_low, -1, -1)
        last_iov = IoV(chunk[-1].exp_low, chunk[-1].run_low, -1, -1)
        if last_iov < output_iov:  # All the chunk iovs are earlier than the requested
            continue
        else:
            input_files = list(chain.from_iterable([list(g) for k, g in groupby(
                input_data, lambda x: input_data[x] in chunk) if k]))
            # Check the minimum number of files in the physics/beam run chunk
            if len(input_files) < min_files_per_chunk:
                continue
            # From the second chunk within the requested range, we have the iov defined by the first run
        specific_iov = first_iov if iCal > 0 else output_iov
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files)} for the output {specific_iov}")
        cal = hot_pixel_mask_calibration(
                cal_name="{}_PXDHotPixelMaskCalibration_BeamorPhysics".format(iCal + 1),
                input_files=input_files,
                **cal_kwargs)
        cal.algorithms[0].params = {"iov_coverage": specific_iov}
        cal_list.append(cal)
        iCal += 1

    # Cosmic run
    nCal_phy = iCal
    chunk_list = chunks_cosmic
    input_data = reduced_file_to_iov_cosmics
    for ichunk, chunk in enumerate(chunk_list):
        first_iov = IoV(chunk[0].exp_low, chunk[0].run_low, chunk[-1].exp_high, chunk[-1].run_high)
        last_iov = IoV(chunk[-1].exp_low, chunk[-1].run_low, chunk[-1].exp_high, chunk[-1].run_high)
        if last_iov < output_iov:  # All the chunk iovs are earlier than the requested
            continue
        # From the first chunk within the requested range, we have the iov defined by the first run
        if iCal == nCal_phy:
            specific_iov = max(first_iov, IoV(
                requested_iov.exp_low, requested_iov.run_low, chunk[-1].exp_high, chunk[-1].run_high))
        else:
            specific_iov = first_iov
        input_files = list(chain.from_iterable([list(g) for k, g in groupby(
            input_data, lambda x: input_data[x] in chunk) if k]))
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files)} for the output {specific_iov}")
        cal = hot_pixel_mask_calibration(
                cal_name="{}_PXDHotPixelMaskCalibration_Cosmic".format(iCal + 1),
                input_files=input_files,
                run_type='cosmic',
                **cal_kwargs)
        cal.algorithms[0].params = {"iov_coverage": specific_iov}  # Not valid when using SimpleRunByRun strategy
        cal_list.append(cal)
        iCal += 1

    # The number of calibrations depends on the 'chunking' above. We would like to make sure that the total number of
    # batch jobs submitted is approximately constant and reasonable, no matter how many files and chunks are used.
    # So we define 1000 total jobs and split this between the calibrations depending on the fraction of total input files
    # in the calibrations.

    total_jobs = 1000
    total_input_files = len(reduced_file_to_iov_physics) + len(reduced_file_to_iov_cosmics)

    for cal in cal_list:
        fraction_of_input_files = len(cal.input_files)/total_input_files
        # Assign the max collector jobs to be roughly the same fraction of total jobs
        cal.max_collector_jobs = ceil(fraction_of_input_files * total_jobs)
        basf2.B2INFO(f"{cal.name} will submit a maximum of {cal.max_collector_jobs} batch jobs")

    return cal_list
