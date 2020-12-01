#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
This module contains various utility functions for the prompt calibration CAF scripts to use.
"""
from basf2 import B2INFO
from collections import defaultdict, OrderedDict
from itertools import groupby
import ROOT
from caf.utils import ExpRun, IoV
from random import choice


def filter_by_max_files_per_run(files_to_iov, max_files_per_run=1, min_events_per_file=0):
    """This function creates a new files_to_iov dictionary by adding files
    until the maximum numbe of files per run is reached. After this no more files
    are added.

    It makes the assumption that the IoV is a single run, and that the exp_low and run_low of the IoV object
    can be used to create the ExpRun fr comparison of whether to add a new input file.

    Parameters:
        files_to_iov (dict): The standard dictionary you might as input to a Calibration. It is of the form

            >>> files_to_iov = {"file_path.root": IoV(1,1,1,1),}

        max_files_per_run (int): The maximum number of files that we will add to the output dictionary for each run in the
            input dictionary.

        min_events_per_file (int): The minimum number of events that is allowed to be in any included file's tree.

    Returns:
        dict: The same style of dict as the input file_to_iov, but filtered down.
    """
    B2INFO(f"Beginning filtering process to only choose {max_files_per_run} file(s) per run.")
    if min_events_per_file:
        B2INFO(f"We also require that each file must have at least {min_events_per_file} events in the tree.")

    # Our dictionary for appending files to and checking the number per run
    run_to_files = defaultdict(list)
    for input_file, file_iov in files_to_iov.items():
        run = ExpRun(exp=file_iov.exp_low, run=file_iov.run_low)
        run_files = run_to_files.get(run, None)
        if not run_files or len(run_files) < max_files_per_run:
            if not min_events_per_file or (min_events_per_file and events_in_basf2_file(input_file) >= min_events_per_file):
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


def group_files_by_iov(files_to_iov):
    """
    Inverts the files_to_iov dictionary to give back a dictionary of IoV -> File list

    Parameters:
        files_to_iov (dict): {"/path/to/file1.root": IoV(1,1,1,1), "/path/to/file2.root": IoV(1,1,1,1)}

    Returns:
        dict: {IoV(1,1,1,1): ["/path/to/file1.root", "/path/to/file2.root"]}
    """
    iov_to_files = OrderedDict()
    for iov, g in groupby(files_to_iov.items(), lambda g: g[1]):
        files = [f[0] for f in g]
        iov_to_files[iov] = files
    return iov_to_files


def filter_by_max_events_per_run(files_to_iov, max_events_per_run, random_select=False):
    """
    This function creates a new files_to_iov dictionary by appending files
    in order until the maximum number of events are reached per run.

    Parameters:
        files_to_iov (dict): {"/path/to/file.root": IoV(1,1,1,1)} type dictionary. Same style as used by the CAF
            for lookup values.
        max_events_per_run (int): The threshold we want to reach but stop adding files if we reach it.
        random_select (bool): true will select random nfile and false will take first nfile.

    Returns:
        dict: The same style of dict as the input files_to_iov, but filtered down.
    """

    # Invert dictionary so that files are grouped against the same IoV
    iov_to_files = group_files_by_iov(files_to_iov)
    # Ready a new dict to contain the reduced lists
    new_iov_to_files = OrderedDict()

    for iov, files in sorted(iov_to_files.items()):
        run = ExpRun(iov.exp_low, iov.run_low)
        total = 0
        remaining_files = files[:]
        chosen_files = []
        while total < max_events_per_run and remaining_files:
            if random_select:
                file_path = choice(remaining_files)
                remaining_files.remove(file_path)
            else:
                file_path = remaining_files.pop(0)
            events = events_in_basf2_file(file_path)
            # Empty files are skipped
            if not events:
                B2INFO(f"No events in {file_path}, skipping...")
                continue
            total += events
            chosen_files.append(file_path)
            B2INFO(f"Choosing input file for {run}: {file_path} and total events so far {total}")

        # Don't bother making empty input list for a Run
        if chosen_files:
            new_iov_to_files[iov] = chosen_files
        else:
            B2INFO(f"No files chosen for {run}")

    # Now go back to files_to_iov dictionary
    new_files_to_iov = OrderedDict()
    for iov, files in new_iov_to_files.items():
        for path in files:
            new_files_to_iov[path] = iov
    return new_files_to_iov


def filter_by_select_max_events_from_files(input_file_list, select_max_events_from_files):
    """
    This function creates a new list by appending random files until
    the maximum number of events are reached per data set.

    Parameters:
        input_file_list (list): ["/path/to/file2.root", "/path/to/file2.root"]
        select_max_events_from_files (int): The threshold we want to reach but stop adding files if we reach it.

    Returns:
        list: The sorted list of random files or empty list of not enought found
    """

    total = 0
    selected_file = []
    while total < select_max_events_from_files:

        if not input_file_list:
            break

        file_path = choice(input_file_list)
        input_file_list.remove(file_path)

        events = events_in_basf2_file(file_path)
        # Empty files are skipped
        if not events:
            B2INFO(f"No events in {file_path}, skipping...")
            continue

        total += events
        selected_file.append(file_path)
        B2INFO(f"Choosing random input file: {file_path} and total events so far {total}")

    # return empty list if request events found
    if total < select_max_events_from_files:
        B2INFO(f"total events {total} are less than requested {select_max_events_from_files}")
        selected_file = []

    return sorted(selected_file)


def events_in_basf2_file(file_path):
    """Does a quick open and return of the number of entries in a basf2 file's tree object.

    Parameters:
        file_path (str): File path to ROOT file

    Returns:
        int: Number of entries in tree.
    """
    f = ROOT.TFile.Open(file_path, "READ")
    events = f.tree.GetEntries()
    f.Close()
    return events
