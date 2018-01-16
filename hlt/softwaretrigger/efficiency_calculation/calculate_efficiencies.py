#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Automated trigger efficiency calculation using gridcontrol for the KEKCC setup.

Calling this script will schedule all jobs for event generation, then for event reconstruction
and later for analysis of the results.

It will output the trigger efficiencies for each channel and the size of the generated raw data files.

In the future, it should also generate plots automatically.

We use gridcontrol for this. To install it, please do a

    pip3 install -r requirements.txt

in this folder.

All former results in the same store location (given by console argument) will be replaced!
"""

from glob import glob
import os
import argparse

from extract import extract_efficiencies, extract_file_sizes, extract_l1_efficiencies
from gridcontrol_helper import write_gridcontrol_file, call_gridcontrol


def generate_events(channels, n_events, n_jobs, storage_location):
    """
    Helper function to call gridcontrol on the generate.py steering file with
    the correct arguments. Will run N jobs per channel to generate.
    """
    parameters = []

    for channel in channels:
        for number in range(n_jobs):
            channel_path = os.path.join(storage_location, channel)
            generated_path = os.path.join(channel_path, "generated")
            output_file = os.path.join(generated_path, f"{number}.root")

            parameter = {"channel": channel, "random_seed": number + 1111, "output_file": output_file,
                         "n_events": n_events}

            # Create output directory
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="generate.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=0)


def run_reconstruction(channels, storage_location):
    """
    Helper function to call gridcontrol on the reconstruct.py steering file with
    the correct arguments. Will run one job per generated file.
    """
    parameters = []

    for channel in channels:
        channel_path = os.path.join(storage_location, channel)
        generated_path = os.path.join(channel_path, "generated")
        for input_file in glob(os.path.join(generated_path, "*.root")):
            # Create output directory
            output_file = input_file.replace("/generated/", "/reconstructed/")
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameter = {"input_file": input_file, "output_file": output_file}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="reconstruct.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=0)


def calculate_efficiencies(channels, storage_location):
    """
    Helper function to call gridcontrol on the analyse.py steering file with
    the correct arguments. Will run one job per reconstructed file.

    Also extracts the final results of file size and trigger efficiencies.
    """
    parameters = []

    for channel in channels:

        channel_path = os.path.join(storage_location, channel)
        reconstructed_path = os.path.join(channel_path, "reconstructed")
        for input_file in glob(os.path.join(reconstructed_path, "[0-9].root")):
            # Create output directory
            output_file = input_file.replace("/reconstructed/", "/analysed/")
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameter = {"input_file": input_file, "output_file": output_file}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="analyse.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=0)

    extract_efficiencies(channels=channels, storage_location=storage_location)
    extract_l1_efficiencies(channels=channels, storage_location=storage_location)
    extract_file_sizes(channels=channels, storage_location=storage_location)


if __name__ == "__main__":
    channels_to_study = [
        "background_only",

        "ee",
        "eemumu",

        "tau",

        "mumu",
        "mumu_kkgen",

        "ee",
        "gg",

        "continuum_ccbar",
        "continuum_uubar",
        "continuum_ddbar",
        "continuum_ssbar",

        "BB_charged",
        "BB_mixed",

        "pipi",
        "pipipi",
    ]

    parser = argparse.ArgumentParser()
    parser.add_argument("storage_location", help="Where to store the results of the calculations.")
    parser.add_argument("--events_per_job", help="Event to generate per job",
                        type=int, default=1000)
    parser.add_argument("--jobs", help="Number of jobs per channel",
                        type=int, default=5)

    args = parser.parse_args()

    # Produce 5*1000 events in each channel
    generate_events(channels=channels_to_study, n_events=args.events_per_job,
                    n_jobs=args.jobs, storage_location=args.storage_location)

    # Reconstruct each channel
    run_reconstruction(channels=channels_to_study, storage_location=args.storage_location)

    # Calculate file size and efficiencies for each channel
    calculate_efficiencies(channels=channels_to_study, storage_location=args.storage_location)
