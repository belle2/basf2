# -*- coding: utf-8 -*-

"""
airflow script for PXD gain calibration.
Author: qingyuan.liu@desy.de
"""

import basf2
from pxd.calibration import gain_calibration
from prompt.utils import filter_by_max_files_per_run, filter_by_max_events_per_run
from prompt import CalibrationSettings
from caf.utils import ExpRun, IoV
from itertools import groupby
from itertools import chain
from math import ceil, inf

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="PXD gain calibration",
                               expert_username="qyliu",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["physics"],
                               # input_data_filters={
                               # "physics": [
                               # "bhabha_all_calib",
                               # "4S", "Continuum", "Scan",
                               # "physics", "Good"]
                               # },
                               expert_config={
                                   "debug": False,
                                   "total_jobs": 1000,
                                   "gain_method": "analytic",
                                   "min_files_per_chunk": 10,
                                   "min_events_per_file": 1000,  # avoid empty files
                                   "max_events_per_run": 4000000,
                                   "max_files_per_run": 20,  # only valid when max_events/run = 0
                                   "payload_boundaries": []
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
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)
    # expert config
    expert_config = kwargs.get("expert_config")
    gain_method = expert_config["gain_method"]
    debug = expert_config["debug"]
    total_jobs = expert_config["total_jobs"]
    max_events_per_run = expert_config["max_events_per_run"]
    max_files_per_run = expert_config["max_files_per_run"]
    min_files_per_chunk = expert_config["min_files_per_chunk"]
    min_events_per_file = expert_config["min_events_per_file"]
    cal_kwargs = expert_config.get("kwargs", {})

    # print all config
    basf2.B2INFO(f"Requested iov: {requested_iov} ")
    basf2.B2INFO(f"Expert config: {expert_config} ")
    # basf2.B2INFO(f"Expert sets payload boundaries are: {expert_config['payload_boundaries']} ")

    # Read input_data
    file_to_iov_physics = input_data["physics"]

    # Reduce data and create calibration instances for different data categories
    cal_list = []
    if max_events_per_run < 0:
        basf2.B2INFO("No file reduction applied.")
        reduced_file_to_iov_physics = file_to_iov_physics
    elif max_events_per_run == 0:
        basf2.B2INFO(f"Reducing to a maximum of {max_files_per_run} files per run.")
        reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics,
                                                                  max_files_per_run, min_events_per_file)
    else:
        basf2.B2INFO(f"Reducing to a maximum of {max_events_per_run} events per run.")
        reduced_file_to_iov_physics = filter_by_max_events_per_run(file_to_iov_physics,
                                                                   max_events_per_run, random_select=True)

    # input_files_physics = list(reduced_file_to_iov_physics.keys())
    input_iov_set_physics = set(reduced_file_to_iov_physics.values())
    exp_set = set([iov.exp_low for iov in input_iov_set_physics])

    # boundaries setting for run chunks (At certain runs, gain was tuned)
    payload_boundaries = [ExpRun(output_iov.exp_low, output_iov.run_low)]
    payload_boundaries.extend([ExpRun(*boundary) for boundary in expert_config["payload_boundaries"]])
    # We don't need run 0 for the first exp as it's handled by output_iov
    payload_boundaries.extend([ExpRun(exp, 0) for exp in sorted(exp_set)[1:]])
    basf2.B2INFO(f"Final Boundaries: {payload_boundaries}")

    # run chunk creation
    chunks_head = payload_boundaries
    chunks_tail = payload_boundaries[1:] + [ExpRun(inf, inf)]
    iov_chunks = [list(g) for k, g in groupby(sorted(input_iov_set_physics),
                                              lambda x: [i for i, j in zip(chunks_head, chunks_tail) if i <= x < j])]

    # Create calibrations from chunks
    input_file_to_iov = reduced_file_to_iov_physics
    iCal = 0
    for ichunk, chunk in enumerate(iov_chunks):
        first_iov = IoV(chunk[0].exp_low, chunk[0].run_low, -1, -1)
        last_iov = IoV(chunk[-1].exp_low, chunk[-1].run_low, -1, -1)
        if last_iov < output_iov:  # All the chunk iovs are earlier than the requested
            continue
        else:
            input_files = list(chain.from_iterable([list(g) for k, g in groupby(
                input_file_to_iov, lambda x: input_file_to_iov[x] in chunk) if k]))
            # Check the minimum number of files in the physics/beam run chunk
            if len(input_files) < min_files_per_chunk:
                basf2.B2WARNING(f"No enough file in sub run chunk [{chunk[0]},{chunk[-1]}]: {len(input_files)},\
but {min_files_per_chunk} required!")
                continue
            # From the second chunk within the requested range, we have the iov defined by the first run
        specific_iov = first_iov if iCal > 0 else output_iov
        basf2.B2INFO(f"Total number of files actually used as input = {len(input_files)} for the output {specific_iov}")
        cal_name = f"{ichunk+1}_PXDAnalyticGainCalibration"
        if (not debug):
            cal = gain_calibration(
                cal_name=cal_name,
                gain_method=gain_method,
                # boundaries=vector_from_runs(payload_boundaries),
                input_files=input_files,
                **cal_kwargs)
            for alg in cal.algorithms:
                alg.params["iov_coverage"] = specific_iov
            cal_list.append(cal)
        else:
            basf2.B2INFO(f"Dry run on Calibration(name={cal_name})")
        iCal += 1

    # The number of calibrations depends on the 'chunking' above. We would like to make sure that the total number of
    # batch jobs submitted is approximately constant and reasonable, no matter how many files and chunks are used.
    # So we define 1000 total jobs and split this between the calibrations depending on the fraction of total input
    # files in the calibrations.

    # total_jobs = expert_config["total_jobs"]
    total_input_files = len(reduced_file_to_iov_physics)

    for cal in cal_list:
        fraction_of_input_files = len(cal.input_files) / total_input_files
        # Assign the max collector jobs to be roughly the same fraction of total jobs
        cal.max_collector_jobs = ceil(fraction_of_input_files * total_jobs)
        basf2.B2INFO(f"{cal.name} will submit a maximum of {cal.max_collector_jobs} batch jobs")

    return cal_list
