# -*- coding: utf-8 -*-

"""Common functions for KLM calibration strategies."""

import basf2
from caf.utils import ExpRun


def calibration_result_string(result):
    """
    Convert calibration result to text message.
    Parameters:
        result (int): Calibration result.
    """
    if (result == 0):
        res = 'successful'
    elif (result == 1):
        res = 'iteration is necessary'
    elif (result == 2):
        res = 'not enough data'
    elif (result == 3):
        res = 'failure'
    elif (result == 4):
        res = 'undefined'
    return res


def get_lowest_exprun(number_of_experiments, experiment_index, run_list,
                      iov_coverage):
    """
    Get lowest experiment and run numbers.
    Parameters:
        number_of_experiments (int): Number of experiments.
        experiment_index (int): 1-based experiment index.
        run_list (list[ExpRun]): List of runs for the selected experiment.
        iov_coverage (IoV): IOV coverage.
    """
    if iov_coverage and experiment_index == 1:
        lowest_exprun = ExpRun(iov_coverage.exp_low, iov_coverage.run_low)
        if lowest_exprun > run_list[0]:
            basf2.B2WARNING(f'The lowest run {run_list[0]} of input data is '
                            f'smaller than the lowest run {lowest_exprun} of '
                            'the requested IOV coverage. The IOV coverage is '
                            'extended.')
            lowest_exprun = run_list[0]
    else:
        lowest_exprun = run_list[0]
        # Start from the beginning of experiments except the first one.
        if (experiment_index > 1):
            lowest_exprun = ExpRun(lowest_exprun.exp, 0)
    return lowest_exprun


def get_highest_exprun(number_of_experiments, experiment_index, run_list,
                       iov_coverage):
    """
    Get highest experiment and run numbers.
    Parameters:
        number_of_experiments (int): Number of experiments.
        experiment_index (int): 1-based experiment index.
        run_list (list[ExpRun]): List of runs for the selected experiment.
        iov_coverage (IoV): IOV coverage.
    """
    if iov_coverage and experiment_index == number_of_experiments:
        highest_exprun = ExpRun(iov_coverage.exp_high, iov_coverage.run_high)
        if (highest_exprun < run_list[-1] and
            not ((iov_coverage.exp_high == -1) or
                 (iov_coverage.exp_high == run_list[-1].exp and
                  iov_coverage.run_high == -1))):
            basf2.B2WARNING(f'The highest run {run_list[-1]} of input data is '
                            f'larger than the highest run {highest_exprun} of '
                            'the requested IOV coverage. The IOV coverage is '
                            'extended.')
            highest_exprun = run_list[-1]
    else:
        highest_exprun = run_list[-1]
        # Extend the IOV to the end of experiments except the last one.
        if (experiment_index < number_of_experiments):
            highest_exprun = ExpRun(highest_exprun.exp, -1)
    return highest_exprun
