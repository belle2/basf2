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

in this folder. If you use a centrally installed basf2, you use to install into your user folder with

    pip3 install --user -r requirements.txt

All former results in the same store location (given by console argument) will be replaced!
"""

from glob import glob
import os
import argparse
import time
import random

from extract import extract_efficiencies, extract_file_sizes, extract_l1_efficiencies
from gridcontrol_helper import write_gridcontrol_swtrigger, call_gridcontrol


def generate_events(channels, n_events, n_jobs, storage_location, local_execution, skip_if_files_exist, phase, shifts,
                    gridcontrol_template_file):
    """
    Helper function to call gridcontrol on the generate.py steering file with
    the correct arguments. Will run N jobs per channel to generate.
    """
    parameters = []
    # store the used shift for every generated root file so this information can be used later
    filename_used_shift = {}

    all_output_files_exist = True

    for channel in channels:
        for number in range(n_jobs):
            for shift in shifts:
                for shift_parameter, shift_values in shift.items():
                    for value in shift_values:
                        channel_path = os.path.join(storage_location, channel)
                        generated_path = os.path.join(channel_path, "generated")
                        filename = "generated_{}.root".format(len(parameters))
                        output_file = os.path.join(generated_path, filename)
                        filename_used_shift.update({filename: (shift_parameter, value)})

                        all_output_files_exist = all_output_files_exist & os.path.isfile(output_file)

                        parameter = {"channel": channel, "random_seed": number + 1111, "output_file": output_file,
                                     "n_events": n_events, "phase": phase, "shift_parameter": shift_parameter,
                                     "shift_value": value}

                        # Create output directory
                        output_dir = os.path.dirname(output_file)

                        if not os.path.exists(output_dir):
                            os.makedirs(output_dir)

                        parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_swtrigger(
        steering_file=os.path.join(os.path.dirname(os.path.realpath(__file__)), "generate.py"),
        parameters=parameters, gridcontrol_template_file=gridcontrol_template_file,
        local_execution=local_execution)

    if skip_if_files_exist and all_output_files_exist:
        print("Output files already exist! Skipping event generation!")
        return filename_used_shift
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=1)

    return filename_used_shift


def run_reconstruction(channels, storage_location, local_execution, phase, roi_filter,
                       gridcontrol_template_file):
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
            output_file = output_file.replace("generated_", "reconstructed_")
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameter = {"input_file": input_file, "output_file": output_file, "phase": phase, "roi_filter": roi_filter}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_swtrigger(
        steering_file=os.path.join(os.path.dirname(os.path.realpath(__file__)), "reconstruct.py"),
        parameters=parameters, gridcontrol_template_file=gridcontrol_template_file,
        local_execution=local_execution)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=1)


def run_hlt_processing(channels, storage_location, local_execution, phase, roi_filter,
                       gridcontrol_template_file, hlt_mode, jobs=4, max_input_files=10):
    """
    Helper function to call gridcontrol on the reconstruct.py steering file with
    the correct arguments. Will run one job per generated file.
    """
    parameters = []
    input_file_list = []
    output_storage_location = os.path.join(storage_location, "hlt_processing")

    if not os.path.exists(output_storage_location):
        os.makedirs(output_storage_location)

    for channel in channels:
        channel_path = os.path.join(storage_location, channel)
        generated_path = os.path.join(channel_path, "generated")

        for input_file in glob(os.path.join(generated_path, "*.root")):
            input_file_list.append(input_file)

    for number in range(jobs):
        # clone list
        this_random = input_file_list[:]
        # shuffle in place
        random.shuffle(this_random)
        this_random = this_random[:max_input_files]

        # stores the memory consumption over events
        mem_statistics_file = os.path.join(output_storage_location, f"{number}_memory.root")

        parameter = {"input_file_list": "#".join(this_random), "phase": phase, "roi_filter": roi_filter,
                     "hlt_mode": hlt_mode,
                     "mem_statistics_file": mem_statistics_file}
        parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_swtrigger(
        steering_file=os.path.join(os.path.dirname(os.path.realpath(__file__)), "hlt_processing.py"),
        parameters=parameters, gridcontrol_template_file=gridcontrol_template_file,
        local_execution=local_execution)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=1)


def calculate_efficiencies(channels, storage_location, local_execution, filename_used_shift, shifts,
                           gridcontrol_template_file):
    """
    Helper function to call gridcontrol on the analyse.py steering file with
    the correct arguments. Will run one job per reconstructed file.

    Also extracts the final results of file size and trigger efficiencies.
    """
    parameters = []

    for channel in channels:
        channel_path = os.path.join(storage_location, channel)
        reconstructed_path = os.path.join(channel_path, "reconstructed")
        for input_file in glob(os.path.join(reconstructed_path, "reconstructed_*.root")):
            # Create output directory
            output_file = input_file.replace("/reconstructed/", "/analysed/")
            output_file = output_file.replace("reconstructed_", "analysed_")
            output_dir = os.path.dirname(output_file)

            # get the original filename from the generation
            filename = os.path.split(input_file)[1]
            filename = filename.replace("reconstructed_", "generated_")
            shift_information = filename_used_shift[filename]

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameter = {"input_file": input_file, "output_file": output_file, "shift_information": shift_information}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_swtrigger(
        steering_file=os.path.join(os.path.dirname(os.path.realpath(__file__)), "analyse.py"),
        parameters=parameters, gridcontrol_template_file=gridcontrol_template_file,
        local_execution=local_execution)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=1)

    extract_efficiencies(channels=channels, storage_location=storage_location,
                         shifts=shifts, filename_used_shift=filename_used_shift)
    extract_l1_efficiencies(channels=channels, storage_location=storage_location,
                            shifts=shifts, filename_used_shift=filename_used_shift)
    extract_file_sizes(channels=channels, storage_location=storage_location)


if __name__ == "__main__":
    channels_to_study = [
        # background only not working atm. because
        # FullSim complains that the MCParticle StoreArray is not registered
        # "background_only",

        "ee",
        "eemumu",
        "eeee",

        "tau",

        "mumu",

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

    shifts_to_study = [
        {"shift_t0": [-70, -35, 0, 35, 70]}
    ]
    parser = argparse.ArgumentParser()
    parser.add_argument("storage_location", help="Where to store the results of the calculations.")
    parser.add_argument("--events_per_job", help="Event to generate per job",
                        type=int, default=1000)
    parser.add_argument("--jobs", help="Number of jobs per channel",
                        type=int, default=5)
    parser.add_argument("--local", help="Execute on the local system and not via batch processing",
                        action="store_true", default=False)
    parser.add_argument("--hlt-stresstest", help="Run hlt stresstest and not efficiency calculation",
                        action="store_true", default=False)
    parser.add_argument("--hlt-mode", help="hlt mode", type=str, default="collision_filter")
    parser.add_argument("--always-generate", help="Always generate events, even if the out files already exist",
                        action="store_true", default=False)
    parser.add_argument("--phase", help="Select the phase of the Belle II Detector. Can be 2 or 3 (default)",
                        type=int, default=3)
    parser.add_argument("--cosmics", action="store_true", help="for cosmics events")
    parser.add_argument("--no-roi-filter", help="Don't apply the Region-Of-Interest filter for the PXD hits",
                        action="store_true", default=False)
    parser.add_argument("--shift", action="store_true", help="Test hlt for varying detector conditions", default=False)
    parser.add_argument("--gridcontrol-template-file", help="Use this file as gridcontrol template",
                        type=str, default="")

    args = parser.parse_args()

    if args.cosmics:
        channels_to_study = ["cosmics"]

    if args.phase == 2:
        print("\n!!!!\n You selected to run with Phase 2 configuration.")
        print("Did you also set BELLE2_BACKGROUND_DIR to phase 2 background?")
        if "BELLE2_BACKGROUND_DIR" in os.environ:
            print("Currently, BELLE2_BACKGROUND_DIR is " + os.environ["BELLE2_BACKGROUND_DIR"])
        else:
            print("Currently, BELLE2_BACKGROUND_DIR is not set at all !")
        print("Continuing in 5 seconds")
        time.sleep(5)

    # better to put the storage location as absolute path, because the jobs will be executed
    # in different folders later
    abs_storage_location = os.path.abspath(args.storage_location)

    # Produce 5*1000 events in each channel and for each single shift
    if not args.shift:
        shifts_to_study = [{"no_shift": [0]}]

    filename_used_shift = generate_events(channels=channels_to_study, n_events=args.events_per_job,
                                          n_jobs=args.jobs, storage_location=abs_storage_location,
                                          local_execution=args.local,
                                          gridcontrol_template_file=args.gridcontrol_template_file,
                                          skip_if_files_exist=not args.always_generate,
                                          phase=args.phase, shifts=shifts_to_study)

    if args.hlt_stresstest:
        run_hlt_processing(channels=channels_to_study, storage_location=abs_storage_location,
                           local_execution=args.local,
                           phase=args.phase,
                           gridcontrol_template_file=args.gridcontrol_template_file,
                           roi_filter=not args.no_roi_filter,
                           hlt_mode=args.hlt_mode)
    else:
        # Reconstruct each channel
        run_reconstruction(channels=channels_to_study, storage_location=abs_storage_location,
                           local_execution=args.local,
                           phase=args.phase,
                           gridcontrol_template_file=args.gridcontrol_template_file,
                           roi_filter=not args.no_roi_filter)

        # Calculate file size and efficiencies for each channel
        calculate_efficiencies(channels=channels_to_study, storage_location=abs_storage_location,
                               gridcontrol_template_file=args.gridcontrol_template_file,
                               local_execution=args.local, filename_used_shift=filename_used_shift, shifts=shifts_to_study)
