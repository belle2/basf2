#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This module contains various utility functions for the prompt calibration CAF scripts to use.
"""

from basf2 import B2INFO, B2WARNING, B2DEBUG
from collections import defaultdict, OrderedDict
from caf.utils import ExpRun, IoV


def filter_by_max_files_per_run(files_to_iov, max_files_per_run=1):
    """This function creates a new files_to_iov dictionary by adding files
    until the maximum numbe of files per run is reached. After this no more files
    are added.

    It makes the assumption that the IoV is a single run, and that the exp_low and run_low of the IoV object
    can be used to create the ExpRun fr comparison of whether to add a new input file.

    Parameters:
        files_to_iov (dict): The standard dictionary you might as input to a Calibration. It is of the form

            >>> files_to_iov = {"file_path.root": IoV(1,1,1,1),}

        max_files_per_run (int): The maximum number of files that we will add to the output dictionary for each run in the
            input dictionary."""
    B2INFO(f"Beginning filtering process to only choose {max_files_per_run} file(s) per run.")
    # Our dictionary for appending files to and checking the number per run
    run_to_files = defaultdict(list)
    for input_file, file_iov in files_to_iov.items():
        run = ExpRun(exp=file_iov.exp_low, run=file_iov.run_low)
        run_files = run_to_files.get(run, None)
        if not run_files or len(run_files) < max_files_per_run:
            B2INFO(f"Choosing input file for {run}: {input_file}")
            run_to_files[run].append(input_file)

    # runs_to_files was useful for looking up number of files per run. But we want to invert this back to a
    # files_to_iov object, just with less files.
    # In case the input dictionary was OrderedDict we keep the ordering.
    # Python's dictionaries are ordered now, but may not always be.
    new_files_to_iov = OrderedDict()
    for run, run_files in run_to_files.items():
        for file_path in run_files:
            # We made the assumption that the IoVs are single runs
            new_files_to_iov[file_path] = IoV(*run, *run)
    return new_files_to_iov
